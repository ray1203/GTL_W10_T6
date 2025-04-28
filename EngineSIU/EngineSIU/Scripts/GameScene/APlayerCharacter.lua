local ReturnTable = {}

local FVector = EngineTypes.FVector
ReturnTable.SpawnTimer = 0.0 -- 타이머
ReturnTable.SpawnInterval = 0.8 
-- BeginPlay: Actor가 처음 활성화될 때 호출
function ReturnTable:BeginPlay()

    print("BeginPlay ", self.Name)
    self.this:SetCharacterMeshCount(1)
    self.SpawnTimer = 0.0
end

-- Tick: 매 프레임마다 호출
function ReturnTable:Tick(DeltaTime)
    self.SpawnTimer = self.SpawnTimer + DeltaTime
    if self.SpawnTimer >= self.SpawnInterval then
        self.SpawnTimer = self.SpawnTimer - self.SpawnInterval 
        local SpawnLoc = self.this.ActorLocation
        print("Failed to spawn Bullet")
        local NewActor = self.this:SpawnActorLua("ABullet", SpawnLoc)

        if NewActor ~= nil then
            print("Spawned Bullet at:", SpawnLoc.X, SpawnLoc.Y, SpawnLoc.Z)
        else
            print("Failed to spawn Bullet")
        end
    end
end

-- EndPlay: Actor가 파괴되거나 레벨이 전환될 때 호출
function ReturnTable:EndPlay(EndPlayReason)
    -- print("[Lua] EndPlay called. Reason:", EndPlayReason)
    print("EndPlay")

end

function ReturnTable:OnOverlapEnemy(Other, Damage)
    print("Other Damage", Damage)
    Other:Destroy()
    self.this.Health = self.this.Health - Damage
    print("Health ", self.this.Health)
end

function ReturnTable:OnOverlapWall(Other, VarientValue)
    print("Wall", VarientValue)
    Other:Destroy()
    self.this:AddCharacterMeshCount(VarientValue)
end

return ReturnTable
