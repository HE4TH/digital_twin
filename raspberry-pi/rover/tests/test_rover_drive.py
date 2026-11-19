"""Offline regression tests: no Paho, broker connection, GPIO, or real motors.

Run from the repository root only after execution is resumed:
python -m unittest discover -s raspberry-pi/rover/tests -v
"""

import importlib.util
from pathlib import Path
import threading
from types import SimpleNamespace
import unittest
from unittest.mock import patch


SPEC = importlib.util.spec_from_file_location(
    "rover_drive_under_test", Path(__file__).resolve().parents[1] / "rover_drive.py")
rover = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(rover)


class RecordingOutput:
    def __init__(self):
        self.writes = []
        self.closed = False

    def write(self, left, right):
        self.writes.append((left, right))

    def close(self):
        self.closed = True


class ReceiverTests(unittest.TestCase):
    def setUp(self):
        self.now = 0.0
        self.output = RecordingOutput()
        self.drive = rover.DriveController(self.output, clock=lambda: self.now)
        self.addCleanup(self.drive.close)
        self.drive.set_ready(True)

    def start_drive(self):
        self.assertTrue(self.drive.receive(b'{"th":0.5,"st":0.25}'))
        self.now += 0.05
        self.assertTrue(self.drive.step()["live"])
        self.assertGreater(self.output.writes[-1][0], 0)
        self.assertAlmostEqual(self.output.writes[-1][0], 0.15)
        self.assertAlmostEqual(self.output.writes[-1][1], 0.05)

    def assert_stopped(self, reason):
        status = self.drive.step()
        self.assertFalse(status["live"])
        self.assertEqual(status["reason"], reason)
        self.assertEqual(self.output.writes[-1], (0.0, 0.0))

    def test_timeout_and_fresh_command(self):
        self.start_drive()
        self.now = 0.299
        self.assertTrue(self.drive.step()["live"])
        self.now = 0.3
        self.assert_stopped("timeout")
        self.now = 0.4
        self.start_drive()

    def test_zero_command_stops_without_waiting_for_timeout(self):
        self.start_drive()
        self.drive.receive(b'{"th":0,"st":0}')
        self.assertTrue(self.drive.step()["live"])
        self.assertEqual(self.output.writes[-1], (0.0, 0.0))

    def test_invalid_packet_clears_previous_drive(self):
        invalid_packets = [
            b'not-json', b'[]', b'null', b'{"th":1}', b'{"st":0}',
            b'{"th":"1","st":0}', b'{"th":true,"st":0}',
            b'{"th":null,"st":0}', b'{"th":NaN,"st":0}',
            b'{"th":Infinity,"st":0}', b'{"th":1e400,"st":0}',
            b'{"th":0,"st":false}', b'{"th":0,"st":{}}',
            b'\xff', b' ' * 1025,
            b'{"th":' + b'9' * 400 + b',"st":0}',
        ]
        for packet in invalid_packets:
            with self.subTest(packet=packet[:60]):
                self.start_drive()
                self.assertFalse(self.drive.receive(packet))
                self.assert_stopped("invalid_command")

    def test_retained_drive_is_rejected_and_cleared(self):
        self.start_drive()
        self.assertFalse(self.drive.receive(b'{"th":1,"st":0}', retained=True))
        self.assert_stopped("retained_command")

    def test_disconnect_and_reconnect_require_fresh_command(self):
        self.start_drive()
        self.drive.set_ready(False)
        self.assertFalse(self.drive.receive(b'{"th":1,"st":0}'))
        self.assert_stopped("not_connected")
        self.drive.set_ready(True)
        self.assert_stopped("waiting_command")
        self.start_drive()

    def test_callbacks_only_update_state_not_motor_output(self):
        self.start_drive()
        count = len(self.output.writes)
        self.drive.receive(b'{"th":-1,"st":0}')
        self.drive.set_ready(False)
        self.assertEqual(len(self.output.writes), count)
        self.assert_stopped("not_connected")

    def test_clamping_and_existing_reverse_mixing(self):
        self.assertEqual(rover.parse_command(b'{"th":5,"st":-2}'), (1.0, -1.0))
        self.drive.receive(b'{"th":-1,"st":0}')
        self.drive.step()
        self.assertEqual(self.output.writes[-1], (-0.2, -0.2))
        for th in (-1.0, 0.0, 1.0):
            for st in (-1.0, 0.0, 1.0):
                self.assertTrue(all(abs(v) <= 0.2 for v in rover.mix_command(th, st, 0.2)))

    def test_shutdown_cannot_be_undone_by_late_callback(self):
        self.start_drive()
        self.drive.close()
        self.drive.set_ready(True)
        self.assertFalse(self.drive.receive(b'{"th":1,"st":0}'))
        self.drive.close()
        self.assertTrue(self.output.closed)
        self.assertEqual(self.output.writes[-1], (0.0, 0.0))
        with self.assertRaises(RuntimeError):
            self.drive.step()

    def test_suback_is_required_and_rejection_stops(self):
        stop = threading.Event()
        callbacks = rover.MqttCallbacks(self.drive, "veh/cmd", stop)
        client = SimpleNamespace(subscribe=lambda *args, **kwargs: (0, 42))
        callbacks.on_connect(client, None, None, 0, None)
        self.assertFalse(self.drive.receive(b'{"th":1,"st":0}'))
        callbacks.on_subscribe(client, None, 41, [SimpleNamespace(is_failure=False)], None)
        self.assert_stopped("not_connected")
        callbacks.on_subscribe(client, None, 42, [SimpleNamespace(is_failure=False)], None)
        self.start_drive()
        callbacks.on_disconnect(client, None, None, 1, None)
        self.assert_stopped("not_connected")
        callbacks.on_connect(client, None, None, 0, None)
        with self.assertLogs("rover_drive", level="ERROR"):
            callbacks.on_subscribe(client, None, 42, [SimpleNamespace(is_failure=True)], None)
        self.assertTrue(stop.is_set())
        self.assertTrue(callbacks.failed)
        self.assert_stopped("not_connected")

    def test_default_mode_has_separate_dry_run_telemetry(self):
        args = rover.parse_args([])
        self.assertEqual(args.max_speed, 0.2)
        with self.assertRaises(ValueError):
            rover.DriveController(RecordingOutput(), max_speed=0.21)
        self.assertTrue(args.dry_run)
        self.assertEqual(args.state_topic, "veh/state/dry-run")
        hardware = rover.parse_args(["--motors", "--max-speed", "0.2"])
        self.assertFalse(hardware.dry_run)
        self.assertEqual(hardware.state_topic, "veh/state")

    def test_connection_refusal_cannot_enable_drive(self):
        self.start_drive()
        callbacks = rover.MqttCallbacks(self.drive, "veh/cmd", threading.Event())
        with self.assertLogs("rover_drive", level="WARNING"):
            callbacks.on_connect(None, None, None, 5, None)
        self.assertFalse(self.drive.receive(b'{"th":1,"st":0}'))
        self.assert_stopped("not_connected")

    def test_unrelated_topics_do_not_change_command(self):
        self.start_drive()
        callbacks = rover.MqttCallbacks(self.drive, "veh/cmd", threading.Event())
        message = SimpleNamespace(topic="home/alert", payload=b'{"th":1,"st":0}', retain=False)
        callbacks.on_message(None, None, message)
        status = self.drive.step()
        self.assertEqual((status["th"], status["st"]), (0.5, 0.25))

    def test_right_polarity_changes_only_hardware_output(self):
        output = rover.MotorOutput.__new__(rover.MotorOutput)
        output.motors = tuple(SimpleNamespace(value=0) for _ in range(4))
        output.invert_right = True
        for left, right in [(0.3,0.3), (-0.3,-0.3), (0.1,0.3), (0,0)]:
            output.write(left, right)
            self.assertEqual(tuple(m.value for m in output.motors), (left,left,-right,-right))
        self.assertTrue(rover.parse_args(["--invert-right"]).invert_right)
        self.assertTrue(rover.parse_args([]).invert_right)
        self.assertFalse(rover.parse_args(["--no-invert-right"]).invert_right)

    def test_verified_chassis_motor_names_and_forward_polarity(self):
        module = SimpleNamespace(**{f"motor{i}": SimpleNamespace(value=0) for i in range(1,5)})
        spec = SimpleNamespace(loader=SimpleNamespace(exec_module=lambda obj: None))
        with patch.object(rover.importlib.util, "spec_from_file_location", return_value=spec), \
             patch.object(rover.importlib.util, "module_from_spec", return_value=module):
            output = rover.MotorOutput(__file__)
        cases = [(0.25,0, (0,0.25,0,0.25)),
                 (0,0.25, (-0.25,0,-0.25,0)),
                 (0.25,0.25, (-0.25,0.25,-0.25,0.25)),
                 (-0.25,-0.25, (0.25,-0.25,0.25,-0.25)),
                 (0,0, (0,0,0,0))]
        for left,right,expected in cases:
            output.write(left,right)
            self.assertEqual(tuple(getattr(module,f"motor{i}").value for i in range(1,5)), expected)

    def test_immediate_drive_pivot_limit_and_immediate_stop(self):
        for i in range(40):
            self.now += 0.05
            self.drive.receive(b'{"th":1,"st":0}')
            self.drive.step()
            self.assertAlmostEqual(self.output.writes[-1][0], 0.2)
        self.drive.receive(b'{"th":0,"st":0}')
        self.drive.step()
        self.assertEqual(self.output.writes[-1], (0,0))
        for i in range(40):
            self.now += 0.05
            self.drive.receive(b'{"th":0,"st":-1}')
            self.drive.step()
        self.assertEqual(self.output.writes[-1], (-0.12,0.12))
        self.now += 0.31
        self.assert_stopped("timeout")

    def test_reverse_ramps_through_zero(self):
        for _ in range(40):
            self.now += 0.05
            self.drive.receive(b'{"th":1,"st":0}')
            self.drive.step()
        self.now += 0.05
        self.drive.receive(b'{"th":-1,"st":0}')
        self.drive.step()
        self.assertAlmostEqual(self.output.writes[-1][0],0.18)
        self.drive.receive(b'bad')
        self.assert_stopped("invalid_command")

    def test_pivot_mixing_matches_verified_chassis(self):
        self.assertEqual(rover.mix_command(0, -1, 0.2), (-0.2, 0.2))
        self.assertEqual(rover.mix_command(0, 1, 0.2), (0.2, -0.2))
        self.assertEqual(rover.mix_command(0, 0, 0.2), (0, 0))

    def test_motor_cleanup_attempts_every_motor_after_failure(self):
        calls = []

        class FakeMotor:
            def __init__(self, index):
                self.index = index

            def stop(self):
                calls.append((self.index, "stop"))
                if self.index == 0:
                    raise RuntimeError("simulated GPIO error")

            def close(self):
                calls.append((self.index, "close"))

        output = rover.MotorOutput.__new__(rover.MotorOutput)
        output.motors = tuple(FakeMotor(index) for index in range(4))
        with self.assertRaises(RuntimeError):
            output.close()
        self.assertEqual(len(calls), 8)
        self.assertIn((3, "close"), calls)


if __name__ == "__main__":
    unittest.main()
