import unreal
import configparser
import os

REBERU_CONTENT_FULL_PATH = f'{unreal.Paths.project_content_dir()}Reberu'
REBERU_SETTINGS : unreal.ReberuSettings = unreal.ReberuSettings.get_default_object()
EAL = unreal.EditorAssetLibrary
ELL = unreal.EditorLevelLibrary
EAS = unreal.get_editor_subsystem(unreal.EditorAssetSubsystem)
ASSET_TOOLS = unreal.AssetToolsHelpers.get_asset_tools()
DA_FACTORY = unreal.DataAssetFactory()


def create_room_data(room_bounds : unreal.RoomBounds, room_name):
    """
    Creates a room data asset or updates the fields on the existing asset if it already exists.
    :param room_bounds: an instance of BP_RoomBounds that contains our room info
    :param room_name: the name of the room to create
    """
    content_path = REBERU_SETTINGS.get_editor_property('reberu_path')
    asset_path = f"{content_path}/Rooms/DA_{room_name}"
    print(f'Creating Reberu Room asset with name {room_name} at {asset_path}')
    does_asset_exist = EAS.does_asset_exist(asset_path)
    if(does_asset_exist):
        unreal.log_warning(f'Asset already exists at: {asset_path}.')
        new_da : unreal.ReberuRoomData = EAS.load_asset(asset_path)
        EAS.checkout_loaded_asset(new_da)
    else:
        new_da : unreal.ReberuRoomData = ASSET_TOOLS.create_asset(f'DA_{room_name}', f'{content_path}/Rooms', unreal.ReberuRoomData, DA_FACTORY)
    if not new_da:
        unreal.log_error(f'Failed to create new data asset at path {content_path}/Rooms/DA_{room_name}')
        return False
    new_da.set_editor_property("RoomName", room_name)
    new_da.room = room_bounds.room
    new_da.room.box_extent = room_bounds.room_box.box_extent
    new_da.room.box_actor_transform = room_bounds.get_actor_transform()
    new_da.room.level = room_bounds.get_world()
    EAL.save_loaded_asset(new_da, False)
    ELL.destroy_actor(room_bounds)
    return True
    
def restore_room_bounds(room_data : unreal.ReberuRoomData, bp_room_class):
    """
    Creates a roombounds bp instance in the current level using the inputted data asset.
    :param room_data: room data asset to grab information from
    """
    new_actor : unreal.RoomBounds = ELL.spawn_actor_from_class(bp_room_class, room_data.room.box_actor_transform.translation, room_data.room.box_actor_transform.rotation.rotator())
    if not new_actor:
        unreal.log_error('failed to create new room bounds from room_data')
        return
    new_actor.room_box.set_box_extent(room_data.room.box_extent)
    new_actor.room = room_data.room
    return new_actor.get_name()

def regenerate_door_ids(room_data : unreal.ReberuRoomData):
    """
    Regenerates all ids for the doors on the inputted room asset.
    :param room_data: room data asset to grab information from
    """
    if not room_data:
        return False
    EAS.checkout_loaded_asset(room_data)
    unreal.ReberuLibrary.regenerate_door_ids(room_data)
    EAL.save_loaded_asset(room_data)
