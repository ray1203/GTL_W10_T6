setmetatable(_ENV, { __index = EngineTypes })

local ReturnTable = {}

local FVector = EngineTypes.FVector 

function ReturnTable:BeginPlay()

    print("BeginPlay ", self.Name)

end

function ReturnTable:Tick(DeltaTime)
    
    local this = self.this

    local NewLoc = this.ActorLocation 
    this.ActorLocation = NewLoc + FVector(1.0, 0.0, 0.0) * DeltaTime 

end

function ReturnTable:EndPlay(EndPlayReason)
    -- print("[Lua] EndPlay called. Reason:", EndPlayReason) -- EndPlayReason Type 등록된 이후 사용 가능.
    print("EndPlay")

end

function ReturnTable:OnBeginOverlap(OwnerActor, OtherActor)


    print("BeginOverlap Bullet", OtherActor)


end

return ReturnTable
