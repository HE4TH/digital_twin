"""Enable this editor session only. Run with Play stopped; never saves config."""
import unreal as ue

editor = ue.get_editor_subsystem(ue.UnrealEditorSubsystem)
if editor.get_game_world():
    raise RuntimeError("Stop Play before enabling RC output")

# Retire the previous five-minute callback if it exists in this editor.
import builtins
old = getattr(builtins, "_rc_bench_session", None)
if old and old.get("handle") is not None:
    ue.unregister_slate_post_tick_callback(old["handle"])
    builtins._rc_bench_session = None
    ue.EditorPythonScripting.set_keep_python_script_alive(False)

bridge = ue.get_default_object(ue.MqttVehicleBridgeSubsystem)
bridge.set_editor_property("broker_host", "192.168.0.6")
bridge.set_editor_property("cmd_topic", "codex/ue-bench/20261019/cmd")
bridge.set_editor_property("receive_vehicle_commands", False)
bridge.set_editor_property("publish_vehicle_input", True)
bridge.set_editor_property("publish_automatic_return", True)
ue.log("RC_SESSION_ENABLED: fixed chassis only; repeated Play allowed until rc_session_stop.py or editor exit. Pi receiver must be started separately.")
