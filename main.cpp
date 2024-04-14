#define GL_SILENCE_DEPRECATION

#include <SDL.h>
#include <SDL_opengl.h>
#include <SDL_image.h>
#include <vector>
#include <iostream>
#include <algorithm>
#include <cassert>
#include <SDL_mixer.h>

SDL_Window* displayWindow;
bool gameIsRunning = true;

GLuint backgroundTexture1, backgroundTexture2, backgroundTexture3;
GLuint winTexture;
GLuint loseTexture;
GLuint playerTexture;
GLuint squareTexture;
GLuint menuTexture;
Mix_Chunk *jumpSound = nullptr;
bool gameOver = false;
bool gameWon = false;
bool inMenu = true;
bool displayWinScreen = false;

int currentLevel = 1;

float playerX = 0.0f;
float playerY = -0.6f;
float playerSpeed = 0.008f;
float playerVelocityX = 0.0f;
float playerJumpPower = 0.1f;
float gravity = -0.01f;
float playerVelocityY = 0.0f;
bool playerIsJumping = false;

float playerSize = 0.03f;
float enemySize = 0.05f;

float groundLevel = -0.6f;
float platformLevel = -0.3f;
float platformStart = 1.5f;
float platformEnd = 3.0f;

float levelWidth = 5.0f;

int playerLives = 3;

struct Bullet {
    float x, y;
    bool active;
};

std::vector<Bullet> bullets;

struct Enemy {
    float x, y;
    bool active;
    float velocityX;
    float minX, maxX;
};



/**
* Author: [Pablo O'Hop]
* Assignment: Platformer
* Date due: 2024-04-13, 11:59pm
* I pledge that I have completed this assignment without
* collaborating with anyone else, in conformance with the
* NYU School of Engineering Policies and Procedures on
* Academic Misconduct.
**/

/**
 So there are a few things going weird with this code that I am unable to figure out.
 1) my textures just stopped loading correctly and are only loading as single colors for some reason.
 so when you see the background change to a different color, it means you are in a new level. Each level is
 distinguished by where the enemy is, where on level 1 it is on the platform, on level 2 it is on the ground and level 3 both.
 
 2) Also the hitbox for ground enemies is a bit weird, where sometimes they will pass right through and other times they will
 die correctly. not sure why.
 
 3) For some reason, despite all my code matching the lecture, my guy will NOT make a jump sound even if i try. I have been
 travelling all week and I have not had the chance to be able to edit this to the fullest extent so I apologzie for it being rudimentary. You can see my files to know that I do have textures and sounds.
 
 */


std::vector<Enemy> enemies;

GLuint LoadTexture(const char* filepath) {
    SDL_Surface* surface = IMG_Load(filepath);
    if (surface == NULL) {
        assert(false);
    }

    GLuint textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);

    SDL_PixelFormat* format = surface->format;
    GLenum textureFormat = GL_RGB;
    if (format->BytesPerPixel == 4) {
        textureFormat = GL_RGBA;
    }

    glTexImage2D(GL_TEXTURE_2D, 0, textureFormat, surface->w, surface->h, 0, textureFormat, GL_UNSIGNED_BYTE, surface->pixels);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    SDL_FreeSurface(surface);

    return textureID;
}

void loadtextures(){
    backgroundTexture1 = LoadTexture("/Users/pabloohop/Desktop/SDLSimple/SDLSimple/images/level_1_back.png");
    backgroundTexture2 = LoadTexture("/Users/pabloohop/Desktop/SDLSimple/SDLSimple/images/level_2_back.png");
    backgroundTexture3 = LoadTexture("/Users/pabloohop/Desktop/SDLSimple/SDLSimple/images/level_3_back.png");
    winTexture = LoadTexture("/Users/pabloohop/Desktop/SDLSimple/SDLSimple/images/maccombanner.jpg");
    loseTexture = LoadTexture("/Users/pabloohop/Desktop/SDLSimple/SDLSimple/images/mfailed.png");
    playerTexture = LoadTexture("/Users/pabloohop/Desktop/SDLSimple/SDLSimple/images/pureleaf.png");
    squareTexture = LoadTexture("/Users/pabloohop/Desktop/SDLSimple/SDLSimple/images/coca.png");
    menuTexture = LoadTexture("/Users/pabloohop/Desktop/SDLSimple/SDLSimple/images/menu_background.png");
}

void setupLevel(int level) {
    enemies.clear();
    switch (level) {
        case 1:
            enemies.push_back({platformStart + (platformEnd - platformStart) / 2, platformLevel + (2 * enemySize), true, 0.005f, platformStart, platformEnd});
            break;
        case 2:
            enemies.push_back({1.0f, groundLevel + enemySize, true, 0.005f, 0.0f, levelWidth});
            break;
        case 3:
            enemies.push_back({1.0f, groundLevel + enemySize, true, 0.005f, 0.0f, levelWidth});
            enemies.push_back({platformStart + (platformEnd - platformStart) / 2, platformLevel + (2 * enemySize), true, 0.005f, platformStart, platformEnd});
            break;
    }
}

void processInput() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) {
            gameIsRunning = false;
        } else if (event.type == SDL_KEYDOWN) {
            if (inMenu) {
                if (event.key.keysym.sym == SDLK_RETURN) {
                    inMenu = false;
                    setupLevel(currentLevel);
                }
            } else {
                switch (event.key.keysym.sym) {
                    case SDLK_LEFT:
                        playerVelocityX = -playerSpeed;
                        break;
                    case SDLK_RIGHT:
                        playerVelocityX = playerSpeed;
                        break;
                    case SDLK_UP:
                        if (!playerIsJumping) {
                            playerVelocityY = playerJumpPower;
                            playerIsJumping = true;
                            Mix_PlayChannel(-1, jumpSound, 0);
                        }
                        break;
                }
            }
        } else if (event.type == SDL_KEYUP && !inMenu) {
            switch (event.key.keysym.sym) {
                case SDLK_LEFT:
                case SDLK_RIGHT:
                    playerVelocityX = 0.0f;
                    break;
            }
        }
    }
}

void update() {
    if (!inMenu) {
        playerX += playerVelocityX;
        playerY += playerVelocityY;
        playerVelocityY += gravity;

        playerX = std::max(0.0f, std::min(levelWidth, playerX));

        if (playerY <= groundLevel) {
            playerY = groundLevel;
            playerIsJumping = false;
            playerVelocityY = 0.0f;
        } else if (playerY <= platformLevel + 0.1f && playerX >= platformStart && playerX <= platformEnd) {
            if (playerVelocityY < 0) {
                playerY = platformLevel + 0.1f;
                playerIsJumping = false;
                playerVelocityY = 0.0f;
            }
        }

        for (auto& enemy : enemies) {
            enemy.x += enemy.velocityX;
            if (enemy.x < enemy.minX || enemy.x > enemy.maxX) {
                enemy.velocityX *= -1;
            }
        }

        for (auto& enemy : enemies) {
            if (enemy.active && std::abs(playerX - enemy.x) < enemySize) {
                float playerBottom = playerY - playerSize;
                float enemyTop = enemy.y + enemySize;
                
                if (playerVelocityY < 0 && playerBottom >= enemyTop) {
                    enemy.active = false;
                } else if (std::abs(playerY - enemy.y) < enemySize) {
                    if (playerLives > 0 && !playerIsJumping) {
                        playerLives--;
                        playerX = 0.0f;
                        if (playerLives == 0) {
                            gameOver = true;
                            gameWon = false;
                            break;
                        }
                    }
                }
            }
        }

        if (!gameOver && std::all_of(enemies.begin(), enemies.end(), [](const Enemy& enemy) { return !enemy.active; })) {
            currentLevel++;
            if (currentLevel > 3) {
                gameOver = true;
                gameWon = true;
                displayWinScreen = true;
            } else {
                setupLevel(currentLevel);
            }
        }
    }
}

void renderMenu() {
    int windowWidth, windowHeight;
    SDL_GetWindowSize(displayWindow, &windowWidth, &windowHeight);
    glClear(GL_COLOR_BUFFER_BIT);

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, menuTexture);
    glBegin(GL_QUADS);
    float aspectRatio = (float)windowWidth / (float)windowHeight;
    glTexCoord2f(0, 1); glVertex2f(-aspectRatio, -1);
    glTexCoord2f(1, 1); glVertex2f(aspectRatio, -1);
    glTexCoord2f(1, 0); glVertex2f(aspectRatio, 1);
    glTexCoord2f(0, 0); glVertex2f(-aspectRatio, 1);
    glEnd();
    glDisable(GL_TEXTURE_2D);

    SDL_GL_SwapWindow(displayWindow);
}

void renderGame() {
    int windowWidth, windowHeight;
    SDL_GetWindowSize(displayWindow, &windowWidth, &windowHeight);
    glClear(GL_COLOR_BUFFER_BIT);

    if (displayWinScreen) {
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, winTexture);
        glBegin(GL_QUADS);
        glVertex2f(-1.0, -1.0);
        glVertex2f(1.0, -1.0);
        glVertex2f(1.0, 1.0);
        glVertex2f(-1.0, 1.0);
        glEnd();
        glDisable(GL_TEXTURE_2D);
        SDL_GL_SwapWindow(displayWindow);
        SDL_Delay(3000);
        gameIsRunning = false;
        return;
    }
    
    float cameraX = std::max(0.0f, std::min(levelWidth - 2.0f, playerX - 1.0f));
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(cameraX, cameraX + 2.0f, -1.0, 1.0, -1.0, 1.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    GLuint currentBackgroundTexture;
    switch (currentLevel) {
        case 1: currentBackgroundTexture = backgroundTexture1; break;
        case 2: currentBackgroundTexture = backgroundTexture2; break;
        case 3: currentBackgroundTexture = backgroundTexture3; break;
        default: currentBackgroundTexture = backgroundTexture1; break;
    }

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, currentBackgroundTexture);
    glBegin(GL_QUADS);
    glVertex2f(0.0f, -1.0f);
    glVertex2f(levelWidth, -1.0f);
    glVertex2f(levelWidth, 1.0f);
    glVertex2f(0.0f, 1.0f);
    glEnd();
    glDisable(GL_TEXTURE_2D);

    if (currentLevel == 1 || currentLevel == 3 || currentLevel == 2) {
        glColor3f(0.5, 0.5, 0.5);
        glBegin(GL_QUADS);
        glVertex2f(platformStart, platformLevel + 0.1f);
        glVertex2f(platformEnd, platformLevel + 0.1f);
        glVertex2f(platformEnd, platformLevel);
        glVertex2f(platformStart, platformLevel);
        glEnd();
    }

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, playerTexture);
    glBegin(GL_QUADS);
    glVertex2f(playerX - playerSize, playerY - playerSize);
    glVertex2f(playerX + playerSize, playerY - playerSize);
    glVertex2f(playerX + playerSize, playerY + playerSize);
    glVertex2f(playerX - playerSize, playerY + playerSize);
    glEnd();
    glDisable(GL_TEXTURE_2D);

    for (const auto& enemy : enemies) {
        if (enemy.active) {
            glEnable(GL_TEXTURE_2D);
            glBindTexture(GL_TEXTURE_2D, squareTexture);
            glBegin(GL_QUADS);
            glVertex2f(enemy.x - enemySize, enemy.y - enemySize);
            glVertex2f(enemy.x + enemySize, enemy.y - enemySize);
            glVertex2f(enemy.x + enemySize, enemy.y + enemySize);
            glVertex2f(enemy.x - enemySize, enemy.y + enemySize);
            glEnd();
            glDisable(GL_TEXTURE_2D);
        }
    }

    SDL_GL_SwapWindow(displayWindow);
}


int main(int argc, char* argv[]) {
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) {
        std::cerr << "Error: " << SDL_GetError() << std::endl;
        return -1;
    }

    displayWindow = SDL_CreateWindow("Platformer", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 480, SDL_WINDOW_OPENGL);
    SDL_GLContext context = SDL_GL_CreateContext(displayWindow);
    SDL_GL_MakeCurrent(displayWindow, context);
    
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(-1.0, 1.0, -1.0, 1.0, -1.0, 1.0);
    glMatrixMode(GL_MODELVIEW);
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
        std::cerr << "Error " << Mix_GetError() << std::endl;
        return -1;
    }

    Mix_Music *bgMusic = Mix_LoadMUS("/Users/pabloohop/Desktop/SDLSimple/SDLSimple/images/game_ost.mp3");
    if (!bgMusic) {
        std::cerr << "Error " << Mix_GetError() << std::endl;
        return -1;
    }
    
    Mix_PlayMusic(bgMusic, -1);
    
    Mix_Chunk *jumpSound = Mix_LoadWAV("/Users/pabloohop/Desktop/SDLSimple/SDLSimple/images/jumping.mp3");
    if (!jumpSound) {
        std::cerr << " Error: " << Mix_GetError() << std::endl;
        return -1;
    }

    
    setupLevel(currentLevel);
    loadtextures();

    while (gameIsRunning) {
        processInput();
        if (inMenu) {
            renderMenu();
        } else {
            update();
            renderGame();
        }
    }

    glDeleteTextures(1, &backgroundTexture1);
    glDeleteTextures(1, &backgroundTexture2);
    glDeleteTextures(1, &backgroundTexture3);
    glDeleteTextures(1, &winTexture);
    glDeleteTextures(1, &loseTexture);
    glDeleteTextures(1, &playerTexture);
    glDeleteTextures(1, &squareTexture);
    glDeleteTextures(1, &menuTexture);
    Mix_FreeMusic(bgMusic);
    Mix_FreeChunk(jumpSound);
    jumpSound = nullptr;
    Mix_CloseAudio();
    SDL_Quit();
    return 0;
}
