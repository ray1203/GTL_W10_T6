local ReturnTable = {} -- Return용 table. cpp에서 Table 단위로 객체 관리.

local FVector = EngineTypes.FVector -- EngineTypes로 등록된 FVector local로 선언.

ReturnTable.LifeTimer = 0.0 -- 수명 타이머 추가

-- BeginPlay: Actor가 처음 활성화될 때 호출
function ReturnTable:BeginPlay()
    print("BeginPlay ", self.Name)
    self.this.Speed = 20
    self.this.AttackDamage = 100
    self.LifeTimer = 0.0
end

-- Tick: 매 프레임마다 호출
function ReturnTable:Tick(DeltaTime)
    -- 이동
    local NewLoc = self.this.ActorLocation
    NewLoc.X = NewLoc.X - self.this.Speed * DeltaTime
    self.this.ActorLocation = NewLoc

    self.LifeTimer = self.LifeTimer + DeltaTime
    if self.LifeTimer >= 6.0 then
        self.this:Destroy()
    end
end

-- EndPlay: Actor가 파괴되거나 레벨이 전환될 때 호출
function ReturnTable:EndPlay(EndPlayReason)
    print("EndPlay")
end

return ReturnTable
