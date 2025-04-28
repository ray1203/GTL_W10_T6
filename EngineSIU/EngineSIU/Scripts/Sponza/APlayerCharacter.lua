local ReturnTable = {}

local FVector = EngineTypes.FVector

-- BeginPlay: Actor가 처음 활성화될 때 호출
function ReturnTable:BeginPlay()

    print("BeginPlay ", self.Name)
    self.this:SetCharacterMeshCount(1)
end

-- Tick: 매 프레임마다 호출
function ReturnTable:Tick(DeltaTime)

end

-- EndPlay: Actor가 파괴되거나 레벨이 전환될 때 호출
function ReturnTable:EndPlay(EndPlayReason)
    -- print("[Lua] EndPlay called. Reason:", EndPlayReason)
    print("EndPlay")

end

function ReturnTable:OnOverlapEnemy(Other)
    print("Other Damage", Other.Damage)
    Other:Destroy()
    self.this.Health = self.this.Health - Other.Damage
    print("Health ", self.this.Health)
end

function ReturnTable:OnOverlapWall(Other, VarientValue)
    print("Wall", VarientValue)
    Other:Destroy()
    self.this:AddCharacterMeshCount(VarientValue)
end

return ReturnTable
