setmetatable(_ENV, { __index = EngineTypes })

local ReturnTable = {}

local FVector = EngineTypes.FVector

-- BeginPlay: Actor가 처음 활성화될 때 호출
function ReturnTable:BeginPlay()

    print("BeginPlay ", self.Name)
    print(self.this)

end

-- Tick: 매 프레임마다 호출
function ReturnTable:Tick(DeltaTime)

    local this = self.this

    -- print(this)

end

-- EndPlay: Actor가 파괴되거나 레벨이 전환될 때 호출
function ReturnTable:EndPlay(EndPlayReason)
    -- print("[Lua] EndPlay called. Reason:", EndPlayReason)
    print("EndPlay")

end

function ReturnTable:OnOverlap(Other)
    print("\nOverlap")
    Other:Destroy()
    self.Health = self.Health - 10
    print(Other.Health)
    print("Location ", self.this.ActorLocation.X)
    print("Health ", self.Health)

end

return ReturnTable
