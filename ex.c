#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h> // For randomization and timing

#ifdef _WIN32
#include <conio.h>
#include <windows.h>
#else
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
#endif

#define MAX_SHELLS 10
#define MAX_ENEMIES 5

struct Shell {
    int x;
    int y;
    bool active;
};

struct Plane {
    int x;
    int y;
};

struct Shell playerShells[MAX_SHELLS];
struct Shell enemyShells[MAX_SHELLS];
struct Plane playerPlane;
struct Plane enemyPlanes[MAX_ENEMIES];

int score = 0; // Score variable

// Function to check if a key is pressed
int kbhit() {
#ifdef _WIN32
    return _kbhit();
#else
    struct termios oldt, newt;
    int ch;
    int oldf;

    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;

    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);

    oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
    fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);

    ch = getchar();

    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);

    if (ch != EOF) {
        ungetc(ch, stdin);
        return 1;
    }

    return 0;
#endif
}

// Function to initialize the game
void initializeGame() {
    playerPlane.x = 10;
    playerPlane.y = 10;

    for (int i = 0; i < MAX_SHELLS; i++) {
        playerShells[i].active = false;
    }

    for (int i = 0; i < MAX_ENEMIES; i++) {
        enemyPlanes[i].x = -1; // Initialize enemy planes as inactive
        enemyPlanes[i].y = -1;
    }

    // Seed the random number generator with the current time
    srand(time(NULL));
}

// Function to generate a random number between min and max (inclusive)
int getRandomNumber(int min, int max) {
    return rand() % (max - min + 1) + min;
}

// Function to update the game state
void updateGame() {
    if (kbhit()) {
        char input = getchar();
        if (input == 'w' && playerPlane.y > 0) {
            playerPlane.y--;
        } else if (input == 's' && playerPlane.y < 24) {
            playerPlane.y++;
        } else if (input == 'a' && playerPlane.x > 0) {
            playerPlane.x--;
        } else if (input == 'd' && playerPlane.x < 79) {
            playerPlane.x++;
        } else if (input == 'q') {
            exit(0);
        } else if (input == ' ') {
            // Implement player shell firing logic here
            for (int i = 0; i < MAX_SHELLS; i++) {
                if (!playerShells[i].active) {
                    playerShells[i].x = playerPlane.x + 4; // Move the shell forward
                    playerShells[i].y = playerPlane.y + 1;
                    playerShells[i].active = true;
                    break;
                }
            }
        }
    }

    // Update player's shells
    for (int i = 0; i < MAX_SHELLS; i++) {
        if (playerShells[i].active) {
            playerShells[i].x++;

            // Deactivate the shell if it goes out of bounds
            if (playerShells[i].x >= 80) {
                playerShells[i].active = false;
            }
        }
    }

    // Generate random enemy planes every 10 seconds
    static time_t lastSpawnTime = 0;
    time_t currentTime = time(NULL);
    if (currentTime - lastSpawnTime >= 10) {
        for (int i = 0; i < MAX_ENEMIES; i++) {
            if (enemyPlanes[i].x == -1) { // Find an inactive enemy plane
                enemyPlanes[i].x = 79; // Start from the right edge
                enemyPlanes[i].y = getRandomNumber(0, 24); // Random vertical position
                break;
            }
        }
        lastSpawnTime = currentTime;
    }

    // Update enemy planes to move to the left
    for (int i = 0; i < MAX_ENEMIES; i++) {
        if (enemyPlanes[i].x >= 0) { // Only update active enemy planes
            enemyPlanes[i].x--;

            // Check for collision with player's shells
            for (int j = 0; j < MAX_SHELLS; j++) {
                if (playerShells[j].active &&
                    playerShells[j].x >= enemyPlanes[i].x && playerShells[j].x <= enemyPlanes[i].x + 6 &&
                    playerShells[j].y >= enemyPlanes[i].y && playerShells[j].y <= enemyPlanes[i].y + 3) {
                    enemyPlanes[i].x = -1; // Deactivate the enemy plane
                    playerShells[j].active = false; // Deactivate the player shell
                    score += 10; // Increase the score by 10
                }
            }
        }
    }

    // Handle collision logic and game over conditions
    // ...
}

void renderGame() {
    // Clear the screen
    printf("\e[H\e[J");

    // Render player's plane (fighter plane appearance)
    printf("\e[%d;%dH  __o  \n", playerPlane.y, playerPlane.x);
    printf("\e[%d;%dH _ \\<_  \n", playerPlane.y + 1, playerPlane.x);
    printf("\e[%d;%dH (_)>(_) \n", playerPlane.y + 2, playerPlane.x);
    printf("\e[%d;%dH/________\\ \n", playerPlane.y + 3, playerPlane.x);

    // Clear the input bar line separately
    printf("\e[26;0H\e[K");

    // Render player's shells
    for (int i = 0; i < MAX_SHELLS; i++) {
        if (playerShells[i].active) {
            printf("\e[%d;%dH*", playerShells[i].y + 1, playerShells[i].x + 1);
        }
    }

    // Render enemy planes
    for (int i = 0; i < MAX_ENEMIES; i++) {
        if (enemyPlanes[i].x >= 0) { // Only render active enemy planes
            printf("\e[%d;%dH  __o  \n", enemyPlanes[i].y, enemyPlanes[i].x);
            printf("\e[%d;%dH _ \\<_  \n", enemyPlanes[i].y + 1, enemyPlanes[i].x);
            printf("\e[%d;%dH (_)>(_) \n", enemyPlanes[i].y + 2, enemyPlanes[i].x);
            printf("\e[%d;%dH/________\\ \n", enemyPlanes[i].y + 3, enemyPlanes[i].x);
        }
    }

    // Render the score in the lower right corner
    printf("\e[25;70HScore: %d", score);

    fflush(stdout); // Flush the output to ensure it's displayed
}

int main() {
    initializeGame();

    while (1) {
        updateGame();
        renderGame();

#ifdef _WIN32
        Sleep(16); // Adjust this value to achieve 60Hz on Windows (1000ms / 60Hz ≈ 16ms)
#else
        usleep(50000); // Increase the delay for slower enemy movement (50000us ≈ 50ms)
#endif
    }

    return 0;
}
