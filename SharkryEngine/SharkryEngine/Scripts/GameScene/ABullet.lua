setmetatable(_ENV, { __index = EngineTypes })

local ReturnTable = {}

local FVector = EngineTypes.FVector 

ReturnTable.LifeTimer = 0.0 -- 수명 타이머 추가

function ReturnTable:BeginPlay()

    --print("BeginPlay ", self.Name)
    self.LifeTimer = 0.0
end

function ReturnTable:Tick(DeltaTime)

    local this = self.this

    local Speed = 2;
    local NewLoc = this.ActorLocation 
    this.ActorLocation = NewLoc + FVector(10.0, 0.0, 0.0) * DeltaTime * Speed


    self.LifeTimer = self.LifeTimer + DeltaTime
    if self.LifeTimer >= 3.0 then
        self.this:Destroy()
    end

end

function ReturnTable:EndPlay(EndPlayReason)
    -- print("[Lua] EndPlay called. Reason:", EndPlayReason) -- EndPlayReason Type 등록된 이후 사용 가능.
    --print("EndPlay")

end
function ReturnTable:OnBeginOverlap(OtherActor)
    print("BeginOverlap Bullet", OtherActor)
    self.this:Destroy()

end

return ReturnTable
