local M = {}

-- BeginPlay: Actor가 처음 활성화될 때 호출
function M:BeginPlay()
    -- print("[Lua] BeginPlay called for", self:GetActorLabel())
    print("BeginPlay PointLight");

    -- 예시: Actor의 위치 출력
    local loc = self:GetActorLocation()
    print(string.format("Initial Location: (%.2f, %.2f, %.2f)", loc.x, loc.y, loc.z))
end

-- Tick: 매 프레임마다 호출
function M:Tick(DeltaTime)
    -- 예시: Actor를 매 Tick마다 +X 방향으로 이동
    -- 출력


end

-- EndPlay: Actor가 파괴되거나 레벨이 전환될 때 호출
-- function M:EndPlay(EndPlayReason)
--     -- print("[Lua] EndPlay called. Reason:", EndPlayReason)
--     print("EndPlay")

-- end

return M