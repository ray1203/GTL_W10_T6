local ReturnTable = {}

local FVector = EngineType.FVector

-- BeginPlay: Actor가 처음 활성화될 때 호출
function ReturnTable:BeginPlay()
    -- print("[Lua] BeginPlay called for", self:GetActorLabel())
    print("BeginPlay Cube")
    print("BeginPlay Cube2")
    print("self=", self)
    print(AActor:GetUUID())

    print(self:UUID())
    print("BeginPlay Cube3")

    -- local a = FVector(3.0, 3.0, 3.0)
    -- print(a.X, a.Y, a.Z)

    -- local loc = self:GetActorLocation()
    -- print(string.format("Initial Location: (%.2f, %.2f, %.2f)", loc.x, loc.y, loc.z))
end

-- Tick: 매 프레임마다 호출
function ReturnTable:Tick(DeltaTime)
    -- 예시: Actor를 매 Tick마다 +X 방향으로 이동
    -- 출력
    
    -- local loc = self:GetActorLocation()
    -- print(string.format("[Lua] Ticking... New X: %.2f", loc.x))
    -- print("Tick")

end

-- EndPlay: Actor가 파괴되거나 레벨이 전환될 때 호출
-- function ReturnTable:EndPlay(EndPlayReason)
--     -- print("[Lua] EndPlay called. Reason:", EndPlayReason)
--     print("EndPlay")

-- end

return ReturnTable