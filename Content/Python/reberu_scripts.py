import unreal

REBERU_CONTENT_FULL_PATH = f'{unreal.Paths.project_content_dir()}Reberu'
REBERU_CONTENT_PATH = '/Game/Reberu'
EAL = unreal.EditorAssetLibrary
ELL = unreal.EditorLevelLibrary
ASSET_TOOLS = unreal.AssetToolsHelpers.get_asset_tools()
DA_FACTORY = unreal.DataAssetFactory()

def create_room_data(room_bounds : unreal.RoomBounds, room_name):
    """
    Creates a room data asset.
    :param room_bounds: an instance of BP_RoomBounds that contains our room info
    :param room_name: the name of the room to create
    """
    new_da : unreal.ReberuRoomData = ASSET_TOOLS.create_asset(f'DA_{room_name}', f'{REBERU_CONTENT_PATH}/Rooms', unreal.ReberuRoomData, DA_FACTORY)
    new_da.set_editor_property("RoomName", room_name)
    new_da.box_extent = room_bounds.room_box.box_extent
    new_da.box_location = room_bounds.get_actor_location()
    new_da.room_doors = room_bounds.doors
    new_da.room = room_bounds.get_world()
    EAL.save_loaded_asset(new_da)
    ELL.destroy_actor(room_bounds)
    
def restore_room_bounds(room_data : unreal.ReberuRoomData, bp_room_class):
    """
    Creates a roombounds bp instance in the current level using the inputted data asset.
    :param room_data: room data asset to grab information from
    """
    new_actor : unreal.RoomBounds = ELL.spawn_actor_from_class(bp_room_class, room_data.box_location)
    new_actor.room_box.set_box_extent(room_data.box_extent)
    new_actor.doors = room_data.room_doors
    return new_actor.get_name()
