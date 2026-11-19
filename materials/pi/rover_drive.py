#!/usr/bin/env python3
"""veh/cmd {th, st} receiver. Default: dry run, no GPIO import or motor access.

Requires paho-mqtt 2.x. --motors explicitly enables the inspected Pi motor module.
State values are commanded outputs, not measured speed or position.
"""

import argparse
import importlib.util
import json
import logging
import math
import os
from pathlib import Path
import signal
import threading
import time


TIMEOUT = 0.3
CONTROL_PERIOD = 0.05
MAX_SPEED = 0.2
PIVOT_MAX_SPEED = 0.12
ACCEL_RATE = 0.1  # pivot duty per second; manual forward/reverse starts immediately
DECEL_RATE = 0.4
STATE_HZ = 5
LOG = logging.getLogger("rover_drive")


def clamp(value):
    return max(-1.0, min(1.0, value))


def parse_command(payload, retained=False):
    """Require both finite JSON numbers; never coerce strings or booleans."""
    if retained:
        raise ValueError("retained command")
    if not isinstance(payload, bytes) or not 0 < len(payload) <= 1024:
        raise ValueError("invalid payload length/type")
    data = json.loads(payload.decode("utf-8"))
    if not isinstance(data, dict):
        raise ValueError("expected JSON object")
    values = []
    for key in ("th", "st"):
        value = data.get(key)
        if type(value) not in (int, float) or not math.isfinite(value):
            raise ValueError("th/st must be finite numbers")
        values.append(clamp(value))
    return tuple(values)


def mix_command(throttle, steering, max_speed):
    """Preserve the inspected receiver's differential-drive mixing and polarity."""
    return (clamp(throttle + steering) * max_speed,
            clamp(throttle - steering) * max_speed)


class DryRunOutput:
    """No hardware libraries, GPIO, or external motor module are loaded."""

    def write(self, left, right):
        pass

    def close(self):
        pass


class MotorOutput:
    def __init__(self, module_path, invert_right=True):
        self.invert_right = invert_right
        # Verified 2026-10-19: left=2/4 positive forward; right=1/3 negative forward.
        # Load the exact file, never the differently wired exhibition copy.
        path = Path(module_path).expanduser().resolve(strict=True)
        spec = importlib.util.spec_from_file_location("rover_motor_hardware", path)
        module = importlib.util.module_from_spec(spec)
        # The module also owns Robot composites; keep them alive with their motors.
        self.module = module
        self.motors = ()
        try:
            try:
                spec.loader.exec_module(module)
            finally:
                self.motors = tuple(getattr(module, name) for name in
                                    ("motor2", "motor4", "motor1", "motor3")
                                    if hasattr(module, name))
            if len(self.motors) != 4:
                raise ValueError("motor module must define motor1..motor4")
            self.write(0.0, 0.0)
        except Exception:
            self.close()
            raise

    def write(self, left, right):
        right = -right if self.invert_right else right
        for motor, value in zip(self.motors, (left, left, right, right)):
            motor.value = value

    def close(self):
        # A failure on one pin must not skip stopping/releasing the other motors.
        errors = []
        for operation in ("stop", "close"):
            for motor in self.motors:
                try:
                    getattr(motor, operation)()
                except Exception as error:
                    errors.append(error)
        if errors:
            raise RuntimeError("one or more motor cleanup operations failed") from errors[0]


class DriveController:
    """Callbacks store commands; only the main loop writes or closes motors."""

    def __init__(self, output, max_speed=MAX_SPEED, dry_run=True, clock=time.monotonic):
        if not math.isfinite(max_speed) or not 0 < max_speed <= MAX_SPEED:
            raise ValueError(f"max speed must be in (0, {MAX_SPEED}]")
        self.output = output
        self.max_speed = max_speed
        self.dry_run = dry_run
        self.clock = clock
        self.lock = threading.Lock()
        self.ready = False
        self.closed = False
        self.applied = (0.0, 0.0)
        self.last_step = self.clock()
        self._clear("not_connected")

    def _clear(self, reason):
        self.throttle = self.steering = 0.0
        self.received_at = None
        self.reason = reason
        self.applied = (0.0, 0.0)

    def set_ready(self, ready):
        with self.lock:
            if not self.closed:
                self.ready = ready
                # Reconnect/SUBACK never reuses input from the preceding connection.
                self._clear("waiting_command" if ready else "not_connected")

    def receive(self, payload, retained=False):
        try:
            throttle, steering = parse_command(payload, retained)
        except (ValueError, TypeError, OverflowError, RecursionError):
            with self.lock:
                if not self.closed:
                    self._clear("retained_command" if retained else "invalid_command")
            return False
        with self.lock:
            if self.closed or not self.ready:
                return False
            self.throttle, self.steering = throttle, steering
            self.received_at = self.clock()
            self.reason = "command"
        return True

    def step(self):
        with self.lock:
            if self.closed:
                raise RuntimeError("controller is closed")
            if self.received_at is not None:
                age = self.clock() - self.received_at
                if age < 0 or age >= TIMEOUT:
                    self._clear("timeout")
            live = self.ready and self.received_at is not None
            th, st = (self.throttle, self.steering) if live else (0.0, 0.0)
            now = self.clock()
            dt = max(0.0, min(now - self.last_step, 0.1))
            self.last_step = now
            limit = min(self.max_speed, PIVOT_MAX_SPEED) if abs(th) < 1e-6 else self.max_speed
            targets = mix_command(th, st, limit)
            def approach(current, target):
                # Reverse through zero; never jump directly to the opposite polarity.
                if current * target < 0:
                    target = 0.0
                if abs(th) > 1e-6 and abs(target) > abs(current):
                    return target  # Manual drive starts immediately; retain reversal deceleration.
                rate = ACCEL_RATE if abs(target) > abs(current) else DECEL_RATE
                delta = rate * dt
                return current + max(-delta, min(target-current, delta))
            # Existing zero command also represents focus loss/emergency stop.
            # Keep it immediate; only nonzero speed changes are smoothed.
            if not live or (th == 0 and st == 0):
                self.applied = (0.0, 0.0)
            else:
                self.applied = tuple(approach(a,t) for a,t in zip(self.applied,targets))
            left, right = self.applied
            self.output.write(left, right)
            return {"l": round(left, 3), "r": round(right, 3),
                    "th": round(th, 3), "st": round(st, 3), "live": live,
                    "dry_run": self.dry_run, "reason": self.reason}

    def close(self):
        with self.lock:
            if self.closed:
                return
            self.closed = True
            self.ready = False
            self._clear("shutdown")
            try:
                self.output.write(0.0, 0.0)
            finally:
                self.output.close()


class MqttCallbacks:
    """Paho 2.x callback signatures. No GPIO operations in the network thread."""

    def __init__(self, controller, command_topic, stop_event):
        self.controller = controller
        self.command_topic = command_topic
        self.stop_event = stop_event
        self.subscription_mid = None
        self.failed = False

    def fail(self, message):
        self.controller.set_ready(False)
        self.failed = True
        LOG.error(message)
        self.stop_event.set()

    def on_connect(self, client, userdata, flags, reason_code, properties):
        self.controller.set_ready(False)
        self.subscription_mid = None
        if reason_code != 0:
            LOG.warning("MQTT connection rejected: %s", reason_code)
            return
        result, mid = client.subscribe(self.command_topic, qos=0)
        if result != 0:
            self.fail("MQTT subscribe request failed")
            return
        self.subscription_mid = mid

    def on_subscribe(self, client, userdata, mid, reason_codes, properties):
        if mid != self.subscription_mid:
            return
        self.subscription_mid = None
        if len(reason_codes) != 1 or any(code.is_failure for code in reason_codes):
            self.fail("MQTT subscription rejected")
            return
        if not self.stop_event.is_set():
            self.controller.set_ready(True)
            LOG.info("MQTT subscription confirmed: %s", self.command_topic)

    def on_connect_fail(self, client, userdata):
        self.controller.set_ready(False)
        LOG.warning("MQTT connection failed; retrying with motors stopped")

    def on_disconnect(self, client, userdata, flags, reason_code, properties):
        self.subscription_mid = None
        self.controller.set_ready(False)
        LOG.info("MQTT disconnected: %s", reason_code)

    def on_message(self, client, userdata, message):
        if message.topic == self.command_topic and not self.stop_event.is_set():
            self.controller.receive(message.payload, message.retain)


def parse_args(argv=None):
    parser = argparse.ArgumentParser(description=__doc__)
    mode = parser.add_mutually_exclusive_group()
    mode.add_argument("--dry-run", dest="dry_run", action="store_true",
                      help="observe commands without GPIO (default)")
    mode.add_argument("--motors", dest="dry_run", action="store_false",
                      help="enable the inspected Pi motor module")
    parser.set_defaults(dry_run=True)
    parser.add_argument("--broker", default="localhost")
    parser.add_argument("--port", type=int, default=1883)
    parser.add_argument("--command-topic", default="veh/cmd")
    parser.add_argument("--state-topic", help="default: veh/state/dry-run or veh/state")
    parser.add_argument("--max-speed", type=float, default=MAX_SPEED)
    polarity = parser.add_mutually_exclusive_group()
    polarity.add_argument("--invert-right", dest="invert_right", action="store_true",
                          help="verified chassis: reverse motors 1/3 (default)")
    polarity.add_argument("--no-invert-right", dest="invert_right", action="store_false",
                          help="only for a separately calibrated chassis")
    parser.set_defaults(invert_right=True)
    parser.add_argument("--motor-module", default="~/pi-rover/ch06_basic_control/motor_module.py")
    args = parser.parse_args(argv)
    if not args.broker.strip() or not 1 <= args.port <= 65535:
        parser.error("a broker and TCP port 1..65535 are required")
    if not math.isfinite(args.max_speed) or not 0 < args.max_speed <= MAX_SPEED:
        parser.error(f"--max-speed must be a finite number in (0, {MAX_SPEED}]")
    args.state_topic = args.state_topic or ("veh/state/dry-run" if args.dry_run else "veh/state")
    for topic in (args.command_topic, args.state_topic):
        if not topic or any(char in topic for char in ("+", "#", "\x00")) or len(topic.encode("utf-8")) > 65535:
            parser.error("topics must be nonempty concrete MQTT topics without wildcards")
    if args.command_topic == args.state_topic:
        parser.error("command and state topics must differ")
    return args


def main(argv=None):
    args = parse_args(argv)
    logging.basicConfig(level=logging.INFO, format="%(asctime)s %(levelname)s %(message)s")
    # Importing this file for unit tests does not import Paho or start any activity.
    import paho.mqtt.client as mqtt

    stop_event = threading.Event()
    previous_handlers = {}
    controller = None
    client = None
    loop_started = False
    exit_code = 0

    def request_stop(signum, frame):
        stop_event.set()

    try:
        stop_signals = [signal.SIGINT, signal.SIGTERM]
        if hasattr(signal, "SIGHUP"):
            stop_signals.append(signal.SIGHUP)  # SSH terminal closed on Pi
        for signum in stop_signals:
            previous_handlers[signum] = signal.signal(signum, request_stop)
        output = DryRunOutput() if args.dry_run else MotorOutput(args.motor_module, args.invert_right)
        controller = DriveController(output, args.max_speed, args.dry_run)
        callbacks = MqttCallbacks(controller, args.command_topic, stop_event)
        mode = "dry" if args.dry_run else "motors"
        client = mqtt.Client(mqtt.CallbackAPIVersion.VERSION2,
                             client_id=f"rover-drive-{mode}-{os.getpid()}",
                             clean_session=True, protocol=mqtt.MQTTv311)
        for name in ("on_connect", "on_subscribe", "on_connect_fail", "on_disconnect", "on_message"):
            setattr(client, name, getattr(callbacks, name))
        client.reconnect_delay_set(min_delay=1, max_delay=5)
        client.connect_timeout = 5
        client.max_queued_messages_set(1)
        client.connect_async(args.broker, args.port, keepalive=5)
        if client.loop_start() != mqtt.MQTT_ERR_SUCCESS:
            raise RuntimeError("cannot start MQTT network loop")
        loop_started = True
        LOG.info("mode=%s broker=%s:%d command=%s state=%s max_speed=%.2f",
                 mode, args.broker, args.port, args.command_topic, args.state_topic, args.max_speed)
        last_status = None
        last_publish = 0.0
        while not stop_event.is_set():
            status = controller.step()
            if status != last_status:
                LOG.info("commanded output: %s", json.dumps(status))
                last_status = status
            now = time.monotonic()
            if client.is_connected() and now - last_publish >= 1.0 / STATE_HZ:
                last_publish = now
                client.publish(args.state_topic, json.dumps(status), qos=0, retain=False)
            stop_event.wait(CONTROL_PERIOD)
        exit_code = 1 if callbacks.failed else 0
    except Exception:
        LOG.exception("Receiver failed; stopping outputs")
        exit_code = 1
    finally:
        stop_event.set()
        # Stop/release GPIO before waiting for the networking thread to finish.
        shutdown_reason = "shutdown"
        if controller is not None:
            try:
                controller.close()
            except Exception:
                LOG.exception("Motor cleanup failed; check hardware stop state")
                shutdown_reason = "cleanup_failed"
                exit_code = 1
        if client is not None:
            try:
                if client.is_connected():
                    final_state = {"l": 0.0, "r": 0.0, "th": 0.0, "st": 0.0,
                                   "live": False, "dry_run": args.dry_run,
                                   "reason": shutdown_reason}
                    info = client.publish(args.state_topic, json.dumps(final_state), qos=0, retain=False)
                    info.wait_for_publish(timeout=0.2)
            except Exception:
                LOG.warning("Final state could not be sent; motor cleanup was already attempted")
            try:
                client.disconnect()
            except Exception:
                LOG.exception("MQTT disconnect failed")
                exit_code = 1
            if loop_started:
                try:
                    client.loop_stop()
                except Exception:
                    LOG.exception("MQTT loop cleanup failed")
                    exit_code = 1
        for signum, handler in previous_handlers.items():
            signal.signal(signum, handler)
    return exit_code


if __name__ == "__main__":
    raise SystemExit(main())
