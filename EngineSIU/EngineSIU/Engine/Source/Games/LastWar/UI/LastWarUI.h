#pragma once

class LastWarUI
{

public:
    LastWarUI();
    ~LastWarUI();

    void Initialize();
    void Render();
    void Release();

    void StartGame();
    void GameOver();
    void RestartGame();
  
    static bool bShowGameOver;
};

