/// Je code en C# moi normalement T^T

/* Utilité du programme
Il s'agit d'un jeu avec des inputs, un oiseau qui se déplace et des pipes qui sont spawnés, j'en déduit que c'est un clone de FlappyBird ou un jeu similaire
*/

/* Code Smells, Anti-Patterns et Défauts
- Les conventions de nommages sont illisibles, utilisation de lettres et de chiffres au lieux de noms complets qui éclairent sur l'usage.
- Ce jeu fonctionne avec une GodFunction dans un GodScript. Vu la taille ce n'est pas tres grave que ce soit sur un seul script mais la GodFunction rends le tout illisible.
- Il y a de nombreuses valeurs hardcodées (Magic numbers) alors qu'il serait plus pertinent de les exposer et de leur donner un nom qui éclaire sur leur signification.
- Les commentaires décrivent ce que fait le code de maniere simpliste mais ne donnent aucune information pertinente sur le fonctionnement.
- Il y a des debugs d'erreurs identiques, préférer ajouter un prefix pour signifier quelle partie est en cause ainsi qu'une description le plus détaillées possible de l'erreur pour remonter a sa source.
- Indentations étranges et incohérentes parfois, mais qui se reglent tres facilement avec un formatage automatique.
*/

#include <windows.h>
#include <iostream>
#include <vector>
#include <string>
#include <random>
#include <chrono>
#include <thread>
#include <algorithm>
#include <cmath>
#include <fstream>

int main()
{
    #pragma region Variables

    struct Bird
    {
        float positionY = 9.0f;
        float velocity = 0.0f;
        int top = 0;
        int bottom = 0;
        int left = 10;
        int right = 11;
        bool dead = false;
        float spawnTimer  = 0.0f;
    };

    struct Pipe
    {
        float pipePositionsX;
        int pipeGapTop;
        bool pipeScoredFlag;
    };
    
    // accessible settings
    std::string bestScoreFileName = "best-score.txt";

    // data stored
    std::vector<Pipe> pipesArray;
    Bird bird;

    #pragma endregion


    HANDLE handleInput  = GetStdHandle(STD_INPUT_HANDLE);
    HANDLE handleOutput = GetStdHandle(STD_OUTPUT_HANDLE);
    if (handleInput == INVALID_HANDLE_VALUE)
    {
        std::cerr << "error"  << std::endl; return 1;
    }
    if (handleOutput == INVALID_HANDLE_VALUE)
    {
        std::cerr << "error" << std::endl; return 1;
    }

    DWORD mode  = 0;
    DWORD mode2 = 0;
    DWORD mode3 = 0;
    if (!GetConsoleMode(handleInput, &mode))
    {
        std::cerr << "error" << std::endl; return 1;
    }
    mode2 = mode;
    mode2 &= ~ENABLE_LINE_INPUT;
    mode2 &= ~ENABLE_ECHO_INPUT;
    if (!SetConsoleMode(handleInput, mode2))
    {
        std::cerr << "error" << std::endl; return 1;
    }
    if (GetConsoleMode(handleOutput, &mode3))
    {
        SetConsoleMode(handleOutput, mode3 | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
    }

    unsigned long long currentScore  = 0;
    unsigned long long bestScore = 0;

    int leftHudPadding = 0;
    int rightHudPadding = 0;

    std::mt19937 rng(std::random_device{}());
    std::uniform_int_distribution<int> gapPosition(2, 20 - 6 - 2);
  
    INPUT_RECORD inputRecord;
    DWORD ne = 0;
  
    std::ifstream bestScoreFile(bestScoreFileName);
    if (bestScoreFile)
    {
        bestScoreFile >> bestScore;
        if (!bestScoreFile)
        {
            bestScore = 0;
        }
    }
    bestScoreFile.close();

    auto previousTime = std::chrono::steady_clock::now();
  
    while (!bird.dead)
    {
        auto currentTime = std::chrono::steady_clock::now();
        float deltaTime = std::chrono::duration<float>(currentTime - previousTime).count();
        previousTime = currentTime;
        if (deltaTime > 0.1f)
        {
            deltaTime = 0.1f;
        }
  
        DWORD numberOfEvents = 0;
        if (!GetNumberOfConsoleInputEvents(handleInput, &numberOfEvents))
        {
            SetConsoleMode(handleInput, mode);
            return 1;
        }
  
        for (DWORD i = 0; i < numberOfEvents; ++i)
        {
            if (!ReadConsoleInput(handleInput, &inputRecord, 1, &ne))
            {
                std::cerr << "Failed to read console input." << std::endl;
                SetConsoleMode(handleInput, mode);
                return 1;
            }
      
            if (inputRecord.EventType == KEY_EVENT)
            {
                KEY_EVENT_RECORD keyEvent = inputRecord.Event.KeyEvent;
                if (keyEvent.bKeyDown == TRUE)
                {
                    if (keyEvent.wVirtualKeyCode == VK_RETURN)
                    {
                        bird.velocity = -14.0f;
                    }
                }
            }
        }
  
        bird.velocity = bird.velocity + 42.0f * deltaTime;
        bird.positionY = bird.positionY + bird.velocity * deltaTime;
  
        bird.spawnTimer = bird.spawnTimer + deltaTime;
        if (bird.spawnTimer >= 1.4f)
        {
            bird.spawnTimer = bird.spawnTimer - 1.4f;
            pipesArray.push_back({50.0f, gapPosition(rng), false});
        } 
  
        for (int i = 0; i < (int)pipesArray.size(); i++)
        {
            pipesArray[i].pipePositionsX = pipesArray[i].pipePositionsX - 18.0f * deltaTime;
    
            int pipeRight = (int)std::floor(pipesArray[i].pipePositionsX) + 6 - 1;
            if (!pipesArray[i].pipeScoredFlag && pipeRight < 10)
            {
                pipesArray[i].pipeScoredFlag = true;         
                currentScore = currentScore + 1;        
                if (currentScore > bestScore)
                {
                    bestScore = currentScore;
                }
            }
  
        for (int i = (int)pipesArray.size() - 1; i >= 0; i--)
        {
            if (pipesArray[i].pipePositionsX + 6.0f < 0.0f) 
            {
                pipesArray.erase(pipesArray.begin() + i);
            }
        }
  
        bird.top = (int)std::floor(bird.positionY); 
        bird.bottom = bird.top + 2 - 1;          
        bird.left = 10;
        bird.right = 10 + 2 - 1;
        if (bird.top < 0 || bird.bottom >= 20)
        {
            bird.dead = true;
        }
  
        if (!bird.dead)
        {
            for (int i = 0; i < (int)pipesArray.size(); i++)
            {
                int pipePositionXFloored = (int)std::floor(pipesArray[i].pipePositionsX);
        
                if (bird.right >= pipePositionXFloored && bird.left <= pipePositionXFloored + 6 - 1)
                {
                    for (int y = bird.top; y <= bird.bottom; y++)
                    {
                        if (y < pipesArray[i].pipeGapTop || y >= pipesArray[i].pipeGapTop + 6)
                        {
                            bird.dead = true;
                            break;
                        }
                    }
                }
        
                if (bird.dead) break;
            }
        }
  
        if (bird.dead) break;
    
        std::vector<std::string> frame(20, std::string(50, ' '));
    
        for (int i = 0; i < (int)pipesArray.size(); i++)
        {
            int pipePositionXFloored = (int)std::floor(pipesArray[i].pipePositionsX);
            for (int i = 0; i < 6; i++)
            {
                int x = pipePositionXFloored + i;
                if (x < 0 || x >= 50) continue;
                for (int y = 0; y < 20; y++)
                {
                    if (!(y >= pipesArray[i].pipeGapTop && y < pipesArray[i].pipeGapTop + 6))
                    {
                        frame[y][x] = 'P';
                    }
                }
            }
        }
  
        for (int i = 0; i < 2; i++)
        {
            int y = bird.top + i;
            if (y < 0 || y >= 20) continue;
            for (int y = 0; y < 2; y++)
            {
                int x = 10 + y;
                if (x >= 0 && x < 50)
                {
                    frame[y][x] = 'B';
                }
            }
        }
  
        std::string scoreText = "Score: " + std::to_string(currentScore) + "   Best: " + std::to_string(bestScore);
        if (scoreText.size() > 50)
        {
            scoreText = scoreText.substr(0, 50);
        }
        leftHudPadding = (int)((50 - (int)scoreText.size()) / 2);
        rightHudPadding = 50 - leftHudPadding - (int)scoreText.size();
  
        std::cout << "\x1b[2J\x1b[H";
        std::cout << "+" << std::string(50, '-') << "+" << "\n";
        for (int y = 0; y < 20; y++)
        {
            std::cout << "|";
            for (int x = 0; x < 50; x++)
            {
                char character = frame[y][x];
                if (character == 'P')
                {
                    std::cout << "\x1b[32mP\x1b[0m";
                }
                else if (character == 'B')
                {
                    std::cout << "\x1b[33mB\x1b[0m";
                }
                else
                {
                    std::cout << ' ';
                }
            }
            std::cout << "|\n";
        }
        std::cout << "+" << std::string(50, '-') << "+" << "\n";
        std::cout << "+" << std::string(50, '-') << "+" << "\n";
        std::cout << "|" << std::string(leftHudPadding, ' ') << scoreText << std::string(rightHudPadding, ' ') << "|\n";
        std::cout << "+" << std::string(50, '-') << "+" << "\n";
        std::cout.flush();
  
        float ft = std::chrono::duration<float>(std::chrono::steady_clock::now() - currentTime).count();
        if (ft < 1.0f / 30.0f)
        {
          std::this_thread::sleep_for(std::chrono::duration<float>(1.0f / 30.0f - ft));
        }
    }
  
    std::ofstream fout(bestScoreFileName, std::ios::trunc);
    if (fout)
    {
        fout << bestScore;
    }
    fout.close();
  
    SetConsoleMode(handleInput, mode);
    return 0;
}