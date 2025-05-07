local ReturnTable = {}

local FVector = EngineTypes.FVector
ReturnTable.SpawnTimer = 0.0 -- 타이머
ReturnTable.SpawnInterval = 0.8
-- BeginPlay: Actor가 처음 활성화될 때 호출
function ReturnTable:BeginPlay()

    print("BeginPlay ", self.Name)
    self.this.CharacterMeshCount = 1
    self.SpawnTimer = 0.0
end

-- Tick: 매 프레임마다 호출
function ReturnTable:Tick(DeltaTime)
    self.SpawnTimer = self.SpawnTimer + DeltaTime
    if self.SpawnTimer >= self.SpawnInterval then
        self.SpawnTimer = self.SpawnTimer - self.SpawnInterval 

        local weight = 0.4;
        for i = 0, self.this.CharacterMeshCount - 1 do
        
            local distance = math.sqrt(i) * weight
            local cos = math.cos(i * 100.0) * distance
            local sin = math.sin(i * 100.0) * distance

            local SpawnLoc = self.this.ActorLocation
            --print("Failed to spawn Bullet")
            SpawnLoc.X = SpawnLoc.X + 2 + cos
            SpawnLoc.Y = SpawnLoc.Y - 0.2 + sin
            SpawnLoc.Z = SpawnLoc.Z + 0.7
            local NewActor = self.this:SpawnActorLua("ABullet", SpawnLoc)

            if NewActor ~= nil then
                print("Spawned Bullet at:", SpawnLoc.X, SpawnLoc.Y, SpawnLoc.Z)
            else
                print("Failed to spawn Bullet")
            end
        end
    end
end

-- EndPlay: Actor가 파괴되거나 레벨이 전환될 때 호출
function ReturnTable:EndPlay(EndPlayReason)
    -- print("[Lua] EndPlay called. Reason:", EndPlayReason)
    --print("EndPlay")

end

function ReturnTable:OnOverlapEnemy(Other)
    print("Other Damage", Damage)
    Other:Destroy()
    self.this.Health = self.this.Health - Other.AttackDamage
    print("Health ", self.this.Health)
end

function ReturnTable:OnOverlapWall(Other, VarientValue)
    print("Wall", VarientValue)
    Other:Destroy()
    self.this:AddCharacterMeshCount(VarientValue)
end

return ReturnTable
