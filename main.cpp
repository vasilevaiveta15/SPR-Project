#include <cstdio>
#include <cstdlib>
#include <unistd.h>
#include <pthread.h>
#include <sys/mman.h>
#include <cstring>
#include "Random.cpp"
//заделя “size” пространство в споделената памет,
void *create_shared_memory(size_t size) {
    // Our memory buffer will be readable and writable:
    int protection = PROT_READ | PROT_WRITE;

    // The buffer will be shared (meaning other processes can access it), but
    // anonymous (meaning third-party processes cannot obtain an address for it),
    // so only this process and its children will be able to use it:
    int visibility = MAP_SHARED | MAP_ANONYMOUS;

    // The remaining parameters to `mmap()` are not important for this use case,
    // but the manpage for `mmap` explains their purpose.
    return mmap(nullptr, size, protection, visibility, -1, 0);
}

void *shmem = nullptr;

int points[] = {0, 0, 0};

int isPlay[] = {0, 0, 0};

typedef struct arg_struct {
    int i;
} args;

void *random(void *arguments) {
    args *arg = static_cast<args *>(arguments);
    int *arr = (int *) shmem;
    std::random_device random_device; // create object for seeding
    std::mt19937 engine{random_device()}; // create engine and seed it
    std::uniform_int_distribution<> dist(0, 1); // create distribution for integers with [0-1] range

    arr[arg->i] = dist(engine);
    memcpy(shmem, arr, sizeof(&arr));
    sleep(arr[arg->i]);
    free(arguments);
}

enum Coin {
    EZI, TURA
};

static const char *COIN_STRING[] = {
        "ezi", "tura", "don't play"
};

int findMax(int first, int second) {
    return (first > second) ? first : second;
}

int getPointsForPlayer(int first) {
    if (first == EZI) {
        return 1;
    } else {
        return 0;
    }
}

int *calculatePoints(int first, int second, int third) {
    int firstPoints = 0;
    int secondPoints = 0;
    int thirdPoints = 0;


    if (isPlay[0] == 2) {
        first = 2;
    }

    if (isPlay[1] == 2) {
        second = 2;
    }

    if (isPlay[2] == 2) {
        third = 2;
    }

    if (first == 1) {
        isPlay[0] = 2;
    } else if (first != 2) {
        firstPoints = getPointsForPlayer(first);
        points[0] += firstPoints;
    }

    if (second == 1) {
        isPlay[1] = 2;

    } else if (second != 2) {
        secondPoints = getPointsForPlayer(second);
        points[1] += secondPoints;


    }
    if (third == 1) {
        isPlay[2] = 2;

    } else if (third != 2) {
        thirdPoints = getPointsForPlayer(third);
        points[2] += thirdPoints;
    }


    printf("%s - %s - %s\n", COIN_STRING[first], COIN_STRING[second], COIN_STRING[third]);
    printf("%d - %d - %d\n\n", firstPoints, secondPoints, thirdPoints);

}

void playRound() {
    pthread_t tid[3];
    int coin[3];
    memcpy(shmem, coin, sizeof(coin));

    if (isPlay[0] != 2) {
        //Създава се нишка подава се номера на играча като аргумент и се изчаква да се изпълни
        args *arguments = static_cast<args *>(malloc(sizeof(args)));
        arguments->i = 0;
        pthread_create(&tid[0], nullptr, random, (void *) arguments);
    }
    if (isPlay[1] != 2) {
        args *arguments = static_cast<args *>(malloc(sizeof(args)));
        arguments->i = 1;
        pthread_create(&tid[1], nullptr, random, (void *) arguments);
    }

    if (isPlay[2] != 2) {
        args *arguments = static_cast<args *>(malloc(sizeof(args)));
        arguments->i = 2;
        pthread_create(&tid[2], nullptr, random, (void *) arguments);
    }

    if (isPlay[0] != 2) {
        pthread_join(tid[0], nullptr);

    }

    if (isPlay[1] != 2) {
        pthread_join(tid[1], nullptr);

    }

    if (isPlay[1] != 2) {
        pthread_join(tid[2], nullptr);
    }


    int *newThrow = (int *) shmem;
    calculatePoints(newThrow[0], newThrow[1], newThrow[2]);
}

int main() {
    int p1Points = 0; // end points of players from all games
    int p2Points = 0;
    int p3Points = 0;

    int gameCounter = 1;

    int p1IncomeOrLoose = 0;
    int p2IncomeOrLoose = 0;
    int p3IncomeOrLoose = 0;

    for (;;) {
        printf("Enter the bet of the game:");
        int bet = 10;
        scanf("%d", &bet);
        shmem = create_shared_memory(128);

        for (;;) {
            if (isPlay[0] == 2 && isPlay[1] == 2 && isPlay[2] == 2) {
                int max = findMax(points[0], findMax(points[1], points[2]));
                if (points[0] == points[1] && points[1] == points[2]) {
                    isPlay[0] = 0;
                    isPlay[1] = 0;
                    isPlay[2] = 0;
                } else if (points[0] == points[1] && points[0] == max) {
                    isPlay[0] = 0;
                    isPlay[1] = 0;
                } else if (points[1] == points[2] && points[1] == max) {
                    isPlay[1] = 0;
                    isPlay[2] = 0;
                } else if (points[0] == points[2] && points[0] == max) {
                    isPlay[0] = 0;
                    isPlay[2] = 0;
                } else {
                    break;
                }

            }
            playRound();
        }

        printf("---Game %d Final Result---\n", gameCounter);
        printf("%d - %d - %d\n\n", points[0], points[1], points[2]);

        if (points[0] > points[1] && points[0] > points[2]) {
            printf("Congratulations player 1 win: %d$", bet * 3);
            p1IncomeOrLoose = p1IncomeOrLoose + bet * 2;
            p2IncomeOrLoose = p2IncomeOrLoose - bet;
            p3IncomeOrLoose = p3IncomeOrLoose - bet;
        } else if (points[1] > points[0] && points[1] > points[2]) {
            printf("Congratulations player 2 win: %d$", bet * 3);
            p2IncomeOrLoose = p2IncomeOrLoose + bet * 2;
            p1IncomeOrLoose = p1IncomeOrLoose - bet;
            p3IncomeOrLoose = p3IncomeOrLoose - bet;
        } else {
            printf("Congratulations player 3 win: %d$", bet * 3);
            p3IncomeOrLoose = p3IncomeOrLoose + bet * 2;
            p1IncomeOrLoose = p1IncomeOrLoose - bet;
            p2IncomeOrLoose = p2IncomeOrLoose - bet;
        }

        printf("Do you wanna play another game?\n");
        printf("Type 0 for NO or 1 for YES\n");

        int choice = 0;
        scanf("%d", &choice);

        if (choice == 0) {
            printf("Player 1 balance from the game is %d$\n", p1IncomeOrLoose);
            printf("Player 2 balance from the game is %d$\n", p2IncomeOrLoose);
            printf("Player 3 balance from the game is %d$\n", p3IncomeOrLoose);
            break;
        }

        p1Points += p1Points + points[0];
        p2Points += p2Points + points[1];
        p3Points += p3Points + points[2];

        points[0] = 0;
        points[1] = 0;
        points[2] = 0;

        isPlay[0] = 0;
        isPlay[1] = 0;
        isPlay[2] = 0;

        gameCounter++;
    }
}
