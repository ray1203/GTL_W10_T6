local ReturnTable = {}

local FVector = EngineTypes.FVector

ReturnTable.SpawnTimer = 0.0 -- 타이머
ReturnTable.SpawnInterval = 5.0 -- 5초마다 스폰

function ReturnTable:BeginPlay()
    print("BeginPlay SpawnerActor")

    self.SpawnTimer = 0.0
end

function ReturnTable:Tick(DeltaTime)
    self.SpawnTimer = self.SpawnTimer + DeltaTime

    if self.SpawnTimer >= self.SpawnInterval then
        self.SpawnTimer = self.SpawnTimer - self.SpawnInterval 

        local SpawnLoc = self.this.ActorLocation


        local NewActor = self.this:SpawnActorLua("AEnemyCharacter", SpawnLoc)

        if NewActor ~= nil then
            print("Spawned AEnemyCharacter at:", SpawnLoc.X, SpawnLoc.Y, SpawnLoc.Z)
        else
            print("Failed to spawn AEnemyCharacter")
        end
    end
end

function ReturnTable:EndPlay(EndPlayReason)
    print("EndPlay SpawnerActor")
end

return ReturnTable
