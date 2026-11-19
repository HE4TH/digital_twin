"""Disable future Play sessions and end current Play through normal stop cleanup."""
import unreal as ue
import builtins

bridge = ue.get_default_object(ue.MqttVehicleBridgeSubsystem)
bridge.set_editor_property("publish_vehicle_input", False)
bridge.set_editor_property("publish_automatic_return", False)
bridge.set_editor_property("receive_vehicle_commands", False)
old = getattr(builtins, "_rc_bench_session", None)
if old and old.get("handle") is not None:
    ue.unregister_slate_post_tick_callback(old["handle"])
    builtins._rc_bench_session = None
    ue.EditorPythonScripting.set_keep_python_script_alive(False)
if ue.get_editor_subsystem(ue.UnrealEditorSubsystem).get_game_world():
    ue.get_editor_subsystem(ue.LevelEditorSubsystem).editor_request_end_play()
    ue.log("RC_SESSION_DISABLED: Play stop requested; receiver watchdog remains the fallback if the final zero packet cannot arrive.")
else:
    ue.log("RC_SESSION_DISABLED: no active Play; future Play will not publish vehicle input.")
