local ReturnTable = {}

local FVector = EngineTypes.FVector

ReturnTable.SpawnTimer = 0.0 -- 타이머
ReturnTable.SpawnInterval = 5.0 -- 5초마다 스폰

-- 적 한 번에 최대/최소 스폰 수
ReturnTable.MinSpawnCount = 1
ReturnTable.MaxSpawnCount = 5

-- 반경 반경(월드 유닛 단위) 안에서 랜덤 위치
ReturnTable.SpawnRadius = 3.0

ReturnTable.WallSpawnProbability = 0.5

function ReturnTable:BeginPlay()
    print("BeginPlay SpawnerActor")

    self.SpawnTimer = 0.0
end

function ReturnTable:Tick(DeltaTime)
    self.SpawnTimer = self.SpawnTimer + DeltaTime

    if self.SpawnTimer >= self.SpawnInterval then
        self.SpawnTimer = self.SpawnTimer - self.SpawnInterval 

        local SpawnLoc = self.this.ActorLocation
        local RandomProbability = math.random()

        if RandomProbability < self.WallSpawnProbability then
            local NewActor = self.this:SpawnActorLua("AWall", SpawnLoc)
        else
            -- 1) 한 번에 몇 마리 스폰할지 랜덤 결정
            local spawnCount = math.random(self.MinSpawnCount, self.MaxSpawnCount)
        
            for i = 1, spawnCount do
                -- 2) 원형 반경 안에서 위치 뽑기
                local angle = math.random() * 2 * math.pi
                local offsetX = math.cos(angle) * self.SpawnRadius * math.random()
                local offsetY = math.sin(angle) * self.SpawnRadius * math.random()
                local baseLoc = self.this.ActorLocation
                local spawnLoc = FVector(
                    baseLoc.X + offsetX,
                    baseLoc.Y + offsetY,
                    baseLoc.Z
                )

                -- 3) 실제 스폰
                local NewActor = self.this:SpawnActorLua("AEnemyCharacter", spawnLoc)
                if NewActor then
                    print(string.format("Spawned Enemy #%d at: %.1f, %.1f, %.1f",
                        i, spawnLoc.X, spawnLoc.Y, spawnLoc.Z))
                else
                    print("Failed to spawn Enemy")
                end
            end
        end
    end
end

function ReturnTable:EndPlay(EndPlayReason)
    print("EndPlay SpawnerActor")
end

return ReturnTable
