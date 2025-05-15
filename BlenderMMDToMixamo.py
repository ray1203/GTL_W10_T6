import bpy

def auto_map_full_location_constraints(source_armature, target_armature):
    """Apply 1:1 constraints (Copy Rotation + Copy Location) to all mapped Mixamo bones."""
    special_local_invert_targets = {
        # 왼팔
        "mixamorig:LeftArm", "mixamorig:LeftForeArm", "mixamorig:LeftHand",
        # 양발
        "mixamorig:LeftUpLeg",
        # "mixamorig:LeftLeg", "mixamorig:LeftFoot", "mixamorig:LeftToeBase",
        "mixamorig:RightUpLeg",
        # "mixamorig:RightLeg", "mixamorig:RightFoot", "mixamorig:RightToeBase",
    }
    invert_local_localparent = {
        "mixamorig:LeftUpLeg", "mixamorig:RightUpLeg",
        "mixamorig:LeftFoot", "mixamorig:RightFoot",
    }

    local_world_no_invert = {
        "mixamorig:LeftLeg", "mixamorig:RightLeg",
    }
    legs_all = {
        "mixamorig:LeftUpLeg", "mixamorig:RightUpLeg",
        "mixamorig:LeftLeg", "mixamorig:RightLeg",
        "mixamorig:LeftFoot", "mixamorig:RightFoot",
    }
    left_arm_all = {
        "mixamorig:LeftShoulder", "mixamorig:LeftArm", "mixamorig:LeftForeArm", "mixamorig:LeftHand",
    }

    right_arm_all = {
        "mixamorig:RightShoulder", "mixamorig:RightArm", "mixamorig:RightForeArm", "mixamorig:RightHand",
    }

    # Mapping: mixamo bone -> mmd bone
    mapping = {
        # Root and Spine
        #下半身
        #全ての親
        #センター
        "mixamorig:Hips": "上半身",
        "mixamorig:Spine": "上半身2",
        "mixamorig:Spine1": "上半身2",
        "mixamorig:Spine2": "上半身2",

        # Neck and Head
        "mixamorig:Neck": "首",
        "mixamorig:Head": "頭",

        # Left Arm
        "mixamorig:LeftArm": "腕.L",
        "mixamorig:LeftForeArm": "ひじ.L",
        "mixamorig:LeftHand": "手首.L",

        # Right Arm
        "mixamorig:RightArm": "腕.R",
        "mixamorig:RightForeArm": "ひじ.R",
        "mixamorig:RightHand": "手首.R",

        # Left Leg
        "mixamorig:LeftUpLeg": "足.L",
        "mixamorig:LeftLeg": "ひざ.L",
        "mixamorig:LeftFoot": "足首.L",

        # Right Leg
        "mixamorig:RightUpLeg": "足.R",
        "mixamorig:RightLeg": "ひざ.R",
        "mixamorig:RightFoot": "足首.R",
        
        "mixamorig:LeftShoulder": "肩.L",
        "mixamorig:RightShoulder": "肩.R",
        
#        # Left Fingers
#        "mixamorig:LeftThumbProximal": "親指１.L",
#        "mixamorig:LeftThumbIntermediate": "親指２.L",
#        "mixamorig:LeftThumbDistal": "親指３.L",
#        "mixamorig:LeftIndexProximal": "人指１.L",
#        "mixamorig:LeftIndexIntermediate": "人指２.L",
#        "mixamorig:LeftIndexDistal": "人指３.L",
#        "mixamorig:LeftMiddleProximal": "中指１.L",
#        "mixamorig:LeftMiddleIntermediate": "中指２.L",
#        "mixamorig:LeftMiddleDistal": "中指３.L",
#        "mixamorig:LeftRingProximal": "薬指１.L",
#        "mixamorig:LeftRingIntermediate": "薬指２.L",
#        "mixamorig:LeftRingDistal": "薬指３.L",
#        "mixamorig:LeftLittleProximal": "小指１.L",
#        "mixamorig:LeftLittleIntermediate": "小指２.L",
#        "mixamorig:LeftLittleDistal": "小指３.L",

#        # Right Fingers
#        "mixamorig:RightThumbProximal": "親指１.R",
#        "mixamorig:RightThumbIntermediate": "親指２.R",
#        "mixamorig:RightThumbDistal": "親指３.R",
#        "mixamorig:RightIndexProximal": "人指１.R",
#        "mixamorig:RightIndexIntermediate": "人指２.R",
#        "mixamorig:RightIndexDistal": "人指３.R",
#        "mixamorig:RightMiddleProximal": "中指１.R",
#        "mixamorig:RightMiddleIntermediate": "中指２.R",
#        "mixamorig:RightMiddleDistal": "中指３.R",
#        "mixamorig:RightRingProximal": "薬指１.R",
#        "mixamorig:RightRingIntermediate": "薬指２.R",
#        "mixamorig:RightRingDistal": "薬指３.R",
#        "mixamorig:RightLittleProximal": "小指１.R",
#        "mixamorig:RightLittleIntermediate": "小指２.R",
#        "mixamorig:RightLittleDistal": "小指３.R",
        

    }
    #mapping={}
    # 추가: 매핑되지 않은 본에서 제약 삭제
    # Clean up constraints from unmapped bones
    mapped_bones = set(mapping.keys())
    for bone in source_armature.pose.bones:
        if bone.name not in mapped_bones:
            if bone.constraints:
                print(f"🧹 Removing constraints from unmapped bone: {bone.name}")
                for c in bone.constraints:
                    bone.constraints.remove(c)


    for mixamo_bone, mmd_bone in mapping.items():
        if mixamo_bone not in source_armature.pose.bones:
            print(f"❌ Mixamo bone '{mixamo_bone}' not found.")
            continue
        if mmd_bone not in target_armature.pose.bones:
            print(f"❌ MMD bone '{mmd_bone}' not found.")
            continue

        pbone = source_armature.pose.bones[mixamo_bone]

        # Clear old constraints
        for c in pbone.constraints:
            pbone.constraints.remove(c)
        if mixamo_bone == "mixamorig:Hips":
            # ✅ 위치 복사 (WORLD → WORLD)
            con_loc = pbone.constraints.new('COPY_LOCATION')
            con_loc.target = target_armature
            con_loc.subtarget = mmd_bone
            con_loc.owner_space = 'WORLD'
            con_loc.target_space = 'WORLD'

            # ✅ 회전 복사 (방향은 로컬 기준으로 보정)
            con_rot = pbone.constraints.new('COPY_ROTATION')
            con_rot.target = target_armature
            con_rot.subtarget = mmd_bone
            con_rot.owner_space = 'LOCAL'
            con_rot.target_space = 'LOCAL_WITH_PARENT'
            con_rot.invert_x = True  # ← 보정: Mixamo는 Z forward, MMD는 Y up
            con_rot.invert_z = False
            con_rot.invert_y = False
            print(f"🧍 Hips → {mmd_bone} : Loc(WORLD), Rot(LOCAL_WITH_PARENT)")
        elif mixamo_bone in ["mixamorig:Spine", "mixamorig:Spine1", "mixamorig:Spine2"]:
            con_rot = pbone.constraints.new('COPY_ROTATION')
            con_rot.target = target_armature
            con_rot.subtarget = "上半身2"
            con_rot.owner_space = 'LOCAL_WITH_PARENT'
            con_rot.target_space = 'LOCAL_WITH_PARENT'
            con_rot.invert_x = False
            con_rot.invert_z = False
            con_rot.invert_y = False
        elif mixamo_bone in legs_all:
            # ✅ 다리 회전 (LOCAL_WITH_PARENT ↔ LOCAL_WITH_PARENT) + 반전
            con_rot = pbone.constraints.new('COPY_ROTATION')
            con_rot.target = target_armature
            con_rot.subtarget = mmd_bone
            con_rot.owner_space = 'LOCAL_WITH_PARENT'
            con_rot.target_space = 'LOCAL_WITH_PARENT'
            con_rot.invert_x = True
            con_rot.invert_z = True
#            con_rot.invert_y = False
            print(f"🦵 {mixamo_bone} → {mmd_bone} (leg rot with XZ invert)")
        elif mixamo_bone =="mixamorig:LeftShoulder":
            con_rot = pbone.constraints.new('COPY_ROTATION')
            con_rot.target = target_armature
            con_rot.subtarget = mmd_bone
            con_rot.owner_space = 'LOCAL_WITH_PARENT'
            con_rot.target_space = 'LOCAL_WITH_PARENT'
            con_rot.invert_x = True
            con_rot.invert_z = True
            con_rot.invert_y = False
            con_loc = pbone.constraints.new('COPY_LOCATION')
            con_loc.target = target_armature
            con_loc.subtarget = mmd_bone
            con_loc.owner_space = 'WORLD'
            con_loc.target_space = 'WORLD'
        elif mixamo_bone == "mixamorig:RightShoulder":
            con_rot = pbone.constraints.new('COPY_ROTATION')
            con_rot.target = target_armature
            con_rot.subtarget = mmd_bone
            con_rot.owner_space = 'LOCAL_WITH_PARENT'
            con_rot.target_space = 'LOCAL_WITH_PARENT'
            con_loc = pbone.constraints.new('COPY_LOCATION')
            con_loc.target = target_armature
            con_loc.subtarget = mmd_bone
            con_loc.owner_space = 'WORLD'
            con_loc.target_space = 'WORLD'
        elif mixamo_bone in left_arm_all:
            con_rot = pbone.constraints.new('COPY_ROTATION')
            con_rot.target = target_armature
            con_rot.subtarget = mmd_bone
            con_rot.owner_space = 'LOCAL_WITH_PARENT'
            con_rot.target_space = 'LOCAL_WITH_PARENT'
            con_rot.invert_x = True
            con_rot.invert_z = True
            con_rot.invert_y = False
            print(f"🖐️ {mixamo_bone} → {mmd_bone} (left arm + XZ invert)")

        elif mixamo_bone in right_arm_all:
            con_rot = pbone.constraints.new('COPY_ROTATION')
            con_rot.target = target_armature
            con_rot.subtarget = mmd_bone
            con_rot.owner_space = 'LOCAL_WITH_PARENT'
            con_rot.target_space = 'LOCAL_WITH_PARENT'
            con_rot.invert_x = False
            con_rot.invert_z = False
            con_rot.invert_y = False
            print(f"✋ {mixamo_bone} → {mmd_bone} (right arm normal)")

        else:
            # ✅ 그 외는 기본 회전 (WORLD ↔ WORLD)
            con_rot = pbone.constraints.new('COPY_ROTATION')
            con_rot.target = target_armature
            con_rot.subtarget = mmd_bone
            con_rot.owner_space = 'LOCAL_WITH_PARENT'
            con_rot.target_space = 'LOCAL_WITH_PARENT'
            con_rot.use_x = True
            con_rot.use_y = True
            con_rot.use_z = True
            print(f"✅ {mixamo_bone} → {mmd_bone} (default rot)")

#        con_rot.owner_space = 'WORLD'
#        con_rot.target_space = 'WORLD'
#        if "Left" in mixamo_bone:
#            con_rot.use_x = True
#            con_rot.use_y = False  # ✅ Y축 회전 제외
#            con_rot.use_z = True
#, "mixamorig:Spine", "mixamorig:Spine1", "mixamorig:Spine2"
#        if mixamo_bone in ["mixamorig:Hips"]:
#            # Copy Location
#            con_loc = pbone.constraints.new('COPY_LOCATION')
#            con_loc.target = target_armature
#            con_loc.subtarget = mmd_bone
#            con_loc.owner_space = 'WORLD'
#            con_loc.target_space = 'WORLD'
#        con_rot = pbone.constraints.new('DAMPED_TRACK')
#        con_rot.target = target_armature
#        con_rot.subtarget = mmd_bone
#        con_rot.head_tail = 0.5
#        con_rot.track_axis = 'TRACK_Y'

#        if ".L" in mixamo_bone:
#            con_rot.invert_x = True
#            con_rot.invert_z = True


        print(f"✅ {mixamo_bone} ← {mmd_bone} (with location and rotation)")

# 실행 예시 (Blender 내부에서 사용)
mixamo = bpy.data.objects["Armature"]
mmd = bpy.data.objects["Armature_MMD"]
auto_map_full_location_constraints(mixamo, mmd)
