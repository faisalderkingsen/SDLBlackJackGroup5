#include <stdio.h>  //including libraries
#include <stdbool.h>
#include <math.h>
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include "./constants.h"

// Global variables
///////////////////////////////////////////////////////////////////////////////
int game_is_running = false;
int last_frame_time = 0;
SDL_Window *window = NULL;
SDL_Renderer *renderer = NULL;
TTF_Font *Font;
SDL_Texture *background;
SDL_Texture *startScreen;
SDL_Texture *balanceScreen;
SDL_Texture *betScreen;
SDL_Texture *controls;
SDL_Texture *cards;
SDL_Texture *playerWon;
SDL_Texture *dealerWon;
SDL_Rect bg_src_rect = {0, 0, 800, 600};
SDL_Rect bg_des_rect = {0, 0, 800, 600};
SDL_Rect flipped_card_src_rect = {2 * 79, 4 * 123, 79, 123};
SDL_Rect flipped_card_des_rect = {360, 50, 79, 123};
int game_state = 0, calculate = 0, check = 0;
int bet, balance, balanceCount = 0;
int dealerTotal = 0, playerTotal = 0, dealerCardCount = 0, playerCardCount = 0;
bool dealDone = false, cardHidden = true, stand = false, end = false;


// prototypes
///////////////////////////////////////////////////////////////////////////////
SDL_Texture *initialize_texture_from_file(const char *file_name, SDL_Renderer *renderer);
//The function initialize_texture_from_file loads an image file using SDL_image, creates an SDL texture from it, and returns the texture, with error handling to return NULL in case of failure.


// Declare card struct and arrays
///////////////////////////////////////////////////////////////////////////////
struct card
{
    int suit; // Suit of the caard
    int num; // number or rank of the card
    int value; // Numeric value
};
// Arrays to store Cards
struct card playerCards[10];
struct card dealerCards[10];
// Array to store balnce
  int balancearray[100];


// load an image
///////////////////////////////////////////////////////////////////////////////
SDL_Texture *initialize_texture_from_file(const char *file_name, SDL_Renderer *renderer)
{
    SDL_Surface *image = IMG_Load(file_name); //image load
    SDL_Texture *image_texture = SDL_CreateTextureFromSurface(renderer, image); //create texture
    SDL_FreeSurface(image); //free surface when done
    return image_texture; //
}


// loading  creatwd images on the screen
///////////////////////////////////////////////////////////////////////////////
void Screen()
{
    // background = initialize_texture_from_file("assets/table.jpg", renderer);
    background = initialize_texture_from_file("assets/simpleBackground.png", renderer);
    startScreen = initialize_texture_from_file("assets/startScreen.png", renderer);
    balanceScreen = initialize_texture_from_file("assets/setBalance.png", renderer);
    betScreen = initialize_texture_from_file("assets/betScreen.png", renderer);
    cards = initialize_texture_from_file("assets/cards.png", renderer);
    // playerWon = initialize_texture_from_file("assets/playerWon.png", renderer);
    playerWon = initialize_texture_from_file("assets/playerWonTransparent.png", renderer);
    // dealerWon = initialize_texture_from_file("assets/dealerWon.png", renderer);
    dealerWon = initialize_texture_from_file("assets/dealerWonTransparent.png", renderer);
    controls = initialize_texture_from_file("assets/controls.png", renderer);
}


// display text on screen
///////////////////////////////////////////////////////////////////////////////
void renderText(SDL_Renderer *renderer, const char *text, int x, int y, int size, SDL_Color color)
{
    // Set the font size
    TTF_Font *font = TTF_OpenFont("fonts/Orbitron-VariableFont_wght.ttf", size);
    if (!font)
    {
       
        return;
    }

    // Render the text surface
    SDL_Surface *textSurface = TTF_RenderText_Blended(font, text, color);
    if (!textSurface) //check if could be loaded
    {
        TTF_CloseFont(font);
        // Handle error
        return;
    }

    // convert surface to texture
    SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, textSurface);
    if (!texture)
    {
        SDL_FreeSurface(textSurface);
        TTF_CloseFont(font);
        // Handle error
        return;
    }

    // Set the render draw color to white
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);

    // Set the destination rectangle for the text
    SDL_Rect dstRect = {x, y, textSurface->w, textSurface->h};

    // Render the texture
    SDL_RenderCopy(renderer, texture, NULL, &dstRect);

    // Clean up
    SDL_FreeSurface(textSurface);
    SDL_DestroyTexture(texture);
    TTF_CloseFont(font);
}


//The function isPresent checks whether a specific playing card,
//represented by suit and num, is present in either the player or dealer deck,
//depending on the value of cardType.
///////////////////////////////////////////////////////////////////////////////
bool isPresent(int cardType, int suit, int num)
{ // either player or dealer
    if (cardType == 0)
    {
        for (int i = 0; i < playerCardCount; i++)
        {
            if (playerCards[i].suit == suit && playerCards[i].num == num)
            {
                return true;
            }
        }
        return false;
    }
    else
    {
        for (int i = 0; i < dealerCardCount; i++)
        {
            if (dealerCards[i].suit == suit && dealerCards[i].num == num)
            {
                return true;
            }
        }
        return false;
    }
}

void computeTotal(int cardType, bool hit, bool stand)
{ // either player or dealer
    if (hit == true)  // Update player's total when a new card is drawn
    {
        playerTotal += playerCards[playerCardCount - 1].value;
        printf("Player Total is = %d \n", playerTotal);
    }
    else if (stand == true) // Update dealer's total when standing
    {
        dealerTotal += dealerCards[dealerCardCount - 1].value;
        printf("Dealer Total is = %d \n", dealerTotal);
    }
    else
    {
        if (cardType == 0) // Calculate and display player's total
        {
            // playerTotal = 0;
            for (int i = 0; i < playerCardCount; i++)
            {
                if (playerTotal <= 10 && playerCards[i].value == 1)
                {
                    playerTotal += 11;
                }
                else
                {
                    playerTotal += playerCards[i].value;
                }
            }
        printf("Player Total is = %d \n", playerTotal);
        }
        else
        { // Calculate and display dealer's total, considering hidden card
            dealerTotal = 0;
            for (int i = 0; i <= dealerCardCount; i++)
            {
                if (i == 1 && cardHidden)
                {
                    continue; // Skip hiden card
                }
                else if (dealerTotal <= 10 && dealerCards[i].value == 1)
                {
                    dealerTotal += 11;
                }
                else
                {
                    dealerTotal += dealerCards[i].value;
                }
            }
    printf("Dealer Total is = %d \n", dealerTotal);
        }
    }
}

// Function to calculate the total balance from an array of digits
void countBalance()
{
    balance = 0;
    for (int i = 0; i <= balanceCount; i++)
    {
        balance += balancearray[i] * pow(10, balanceCount - i);
    }
}

// to deal the initial cards
///////////////////////////////////////////////////////////////////////////////
void dealCards()
{
    for (int i = 0; i < 2; i++)
    {
        // deal cards for the player
        int suit = rand() % 4; //colour
        int num = rand() % 13; //Value
        while (isPresent(0, suit, num) == true)
        {
            suit = rand() % 4;
            num = rand() % 13;
        }
        playerCards[i].suit = suit;
        playerCards[i].num = num;
        if (num > 9)
            playerCards[i].value = 10;
        else
            playerCards[i].value = num + 1;
        playerCardCount++;

        // Deal cards for the dealer
        suit = rand() % 4;
        num = rand() % 13;
        while (isPresent(1, suit, num) == true)
        {
            suit = rand() % 4;
            num = rand() % 13;
        }
        dealerCards[i].suit = suit;
        dealerCards[i].num = num;
        if (num > 9)
            dealerCards[i].value = 10;
        else
            dealerCards[i].value = num + 1;
        dealerCardCount++;
    }
}


// restart the game
///////////////////////////////////////////////////////////////////////////////
void reset()
{
    game_state = 2, calculate = 0, check = 0;
    bet = 0, balanceCount = 0;
    dealerTotal = 0, playerTotal = 0, dealerCardCount = 0, playerCardCount = 0;
    dealDone = false, cardHidden = true, stand = false, end = false;
}


// Function to initialize our SDL window
///////////////////////////////////////////////////////////////////////////////
int initialize_window(void)
//The initialize_window function initializes SDL, creates a window titled "BlackJack" with specified dimensions, and sets up a renderer for graphics rendering
{
    if (SDL_Init(SDL_INIT_EVERYTHING) != 0)
    {
        fprintf(stderr, "Error initializing SDL.\n");
        return false;
    }
    window = SDL_CreateWindow(
        "BlackJack",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        WINDOW_WIDTH,
        WINDOW_HEIGHT,
        0);
    if (!window)
    {
        fprintf(stderr, "Error creating SDL Window.\n");
        return false;
    }
    renderer = SDL_CreateRenderer(window, -1, 0);
    if (!renderer)
    {
        fprintf(stderr, "Error creating SDL Renderer.\n");
        return false;
    }
    return true;
}

// Function to poll SDL events and process keyboard input
///////////////////////////////////////////////////////////////////////////////
///

//The process_input function handles SDL events and key presses, interpreting them based on the current game state. It allows the player to set bet amounts, initiate card dealing, and make decisions such as hitting, standing, or resetting the game. The function uses a switch statement to determine the specific action corresponding to the SDL event type and the pressed key.
void process_input(void)
{
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        switch (event.type)
        {
        case SDL_QUIT:
            game_is_running = false;
            break;
        case SDL_KEYDOWN:
            if (event.key.keysym.sym == SDLK_ESCAPE)
            {
                game_is_running = false;
            }
            switch (event.key.keysym.sym)
            {
            case SDLK_1:
                if (game_state == 1)
                {
                    balancearray[balanceCount] = 1;
                    countBalance();
                    balanceCount++;
                }
                if (game_state == 2)
                {
                    if (balance < 10)
                    {
                        printf("Balance is less than the bet. \n");
                    }
                    else
                    {
                        bet = 10;
                        game_state = 3;
                        balance -= bet;
                        printf("My bet is: %d\n", bet);
                        printf("My balance is: %d\n", balance);
                    }
                }
                break;
            case SDLK_2:
                if (game_state == 1)
                {
                    balancearray[balanceCount] = 2;
                    countBalance();
                    balanceCount++;
                }
                if (game_state == 2)
                {
                    if (balance < 100)
                    {
                        printf("Balance is less than the bet. \n");
                    }
                    else
                    {
                        bet = 100;
                        game_state = 3;
                        balance -= bet;
                        printf("My bet is: %d\n", bet);
                        printf("My balance is: %d\n", balance);
                    }
                }
                break;
            case SDLK_3:
                if (game_state == 1)
                {
                    balancearray[balanceCount] = 3;
                    countBalance();
                    balanceCount++;
                }
                if (game_state == 2)
                {
                    if (balance < 1000)
                    {
                        printf("Balance is less than the bet. \n");
                    }
                    else
                    {
                        bet = 1000;
                        game_state = 3;
                        balance -= bet;
                        printf("My bet is: %d\n", bet);
                        printf("My balance is: %d\n", balance);
                    }
                }
                break;
            case SDLK_4:
                if (game_state == 1)
                {
                    balancearray[balanceCount] = 4;
                    countBalance();
                    balanceCount++;
                }
                break;
            case SDLK_5:
                if (game_state == 1)
                {
                    balancearray[balanceCount] = 5;
                    countBalance();
                    balanceCount++;
                }
                break;
            case SDLK_6:
                if (game_state == 1)
                {
                    balancearray[balanceCount] = 6;
                    countBalance();
                    balanceCount++;
                }
                break;
            case SDLK_7:
                if (game_state == 1)
                {
                    balancearray[balanceCount] = 7;
                    countBalance();
                    balanceCount++;
                }
                break;
            case SDLK_8:
                if (game_state == 1)
                {
                    balancearray[balanceCount] = 8;
                    countBalance();
                    balanceCount++;
                }
                break;
            case SDLK_9:
                if (game_state == 1)
                {
                    balancearray[balanceCount] = 9;
                    countBalance();
                    balanceCount++;
                }
                break;
            case SDLK_0:
                if (game_state == 1)
                {
                    balancearray[balanceCount] = 0;
                    countBalance();
                    balanceCount++;
                }
                break;
            case SDLK_RETURN:
                if (game_state == 0)
                {
                    game_state = 1;
                }
                else if (game_state == 1)
                {
                    game_state = 2;
                    // balance = 1000;
                    printf("My balance is: %d\n", balance);
                }
                break;
            case SDLK_d:
                if (game_state == 3 && !dealDone)
                {
                    dealCards();
                    dealDone = true;
                    calculate = 1;
                }
                break;
            case SDLK_h:
                if (game_state == 3 && dealDone && stand == false)
                {
                    int suit = rand() % 4;
                    int num = rand() % 13;
                    while (isPresent(0, suit, num) == true)
                    {
                        suit = rand() % 4;
                        num = rand() % 13;
                    }
                    playerCards[playerCardCount].suit = suit;
                    playerCards[playerCardCount].num = num;
                    if (num > 9)
                        playerCards[playerCardCount].value = 10;
                    else
                        playerCards[playerCardCount].value = num + 1;
                    playerCardCount++;
                    calculate = 3;
                }
                break;
            case SDLK_s:
                if (game_state == 3 && dealDone && !stand)
                {
                    cardHidden = false;
                    stand = true;
                    calculate = 5;
                }
                break;
            case SDLK_r:
                if (check == 3 || game_state == 5)
                {
                    reset();
                }
            default:
                break;
            }
            break;
        }
    }
}


// Setup function that runs once at the beginning of our program
///////////////////////////////////////////////////////////////////////////////
void setup(void)
{
    // background
    IMG_Init(IMG_INIT_JPG | IMG_INIT_PNG);

    // font
    TTF_Init();
}


// Update function with a fixed time step
///////////////////////////////////////////////////////////////////////////////
//The update function calculates and updates the game state based on various conditions. It uses a series of if statements and switches to determine the flow of the game. Key functionalities include computing card totals, determining dealer actions, checking for win/loss conditions, and updating the game state accordingly. The function also considers factors such as player and dealer card totals, blackjack conditions, and end-game scenarios to manage the overall game logic.
void update(void)
{
    // Get delta_time factor converted to seconds to be used to update objects
    float delta_time = (SDL_GetTicks() - last_frame_time) / 1000.0;

    // Store the milliseconds of the current frame to be used in the next one
    last_frame_time = SDL_GetTicks();

    if (calculate == 1)
    {
        calculate = 2;
    }
    else if (calculate == 2)
    {
        computeTotal(0, false, false);
        computeTotal(1, false, false);
        calculate = 0;
    }
    else if (calculate == 3)
    {
        calculate = 4;
    }
    else if (calculate == 4)
    {
        computeTotal(0, true, false);
        calculate = 0;
    }
    else if (calculate == 5)
    {
        calculate = 6;
    }
    else if (calculate == 6)
    {
        computeTotal(1, false, false);
        calculate = 0;
    }
    else if (calculate == 7)
    {
        calculate = 8;
    }
    else if (calculate == 8)
    {
        computeTotal(1, false, true);
        calculate = 0;
    }

    if (stand == true && check == 0)
    {
        check = 1;
    }
    else if (check == 1)
    {
        if (dealerTotal < 17)
        {
            check = 2;
            // printf("Dealer Total is = %d \n", dealerTotal);
        }
        else
        {
            check = 3;
        }
    }
    else if (check == 2)
    {
        int suit = rand() % 4;
        int num = rand() % 13;
        while (isPresent(1, suit, num) == true)
        {
            suit = rand() % 4;
            num = rand() % 13;
        }
        dealerCards[dealerCardCount].suit = suit;
        dealerCards[dealerCardCount].num = num;
        if (num > 9)
            dealerCards[dealerCardCount].value = 10;
        else
            dealerCards[dealerCardCount].value = num + 1;
        dealerCardCount++;
        check = 3;
        calculate = 7;
    }

    if (check == 3)
    {
        if (dealerTotal == 21)
        {
            game_state = 5;
            // printf("dealer won \n");
        }
        if (dealerTotal > 21)
        {
            game_state = 4;
            if (!end)
            {
                balance += bet * 2;
                end = true;
            }
            // printf("player won \n");
        }
        if (dealerTotal > playerTotal)
        {
            game_state = 5;
            // printf("dealer won \n");
        }
        else if (playerTotal > dealerTotal)
        {
            game_state = 4;
            if (!end)
            {
                balance += bet * 2;
                end = true;
            }
            // printf("player won \n");
        }
    }

    if (playerTotal == 21)
    {
        game_state = 4;
        if (!end)
        {
            balance += bet * 2.5;
            end = true;
        }
        // printf("player won \n");
    }
    if (dealerTotal == 21)
    {
        game_state = 5;
        // printf("dealer won \n");
    }
    if (playerTotal > 21)
    {
        game_state = 5;
        // printf("dealer won \n");
    }
    if (dealerTotal > 21)
    {
        game_state = 4;
        if (!end)
        {
            balance += bet * 2;
            end = true;
        }
        // printf("player won \n");
    }
}


// Render function to draw game objects in the SDL window
///////////////////////////////////////////////////////////////////////////////
void render(void)
{
    // Convert balance, playerTotal, and dealerTotal to strings for rendering
    char buffer[10], ptot[10], dtot[10];
    sprintf(buffer, "%d", balance);
    sprintf(ptot, "%d", playerTotal);
    sprintf(dtot, "%d", dealerTotal);
    // Set the draw color to black
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    // Clear the renderer with a black background
    SDL_RenderClear(renderer);
    if (game_state == 0)
        // Render the start screen
        SDL_RenderCopy(renderer, startScreen, &bg_src_rect, &bg_des_rect);
    else if (game_state == 1)
    {
        // Render the balance screen and display the balance text
        SDL_RenderCopy(renderer, balanceScreen, &bg_src_rect, &bg_des_rect);
        renderText(renderer, buffer, 150, 310, 50, {255, 255, 255, 255});
    }
    else if (game_state == 2)
    {
        SDL_RenderCopy(renderer, betScreen, &bg_src_rect, &bg_des_rect);
        renderText(renderer, "Balance:", 0, 0, 20, {255, 255, 255, 255});
        renderText(renderer, buffer, 100, 0, 20, {255, 255, 255, 255});
    }
    else if (game_state == 4)
    {
        // Render the background and the player won screen
        SDL_RenderCopy(renderer, background, &bg_src_rect, &bg_des_rect);
        SDL_RenderCopy(renderer, playerWon, &bg_src_rect, &bg_des_rect);
        // Render texts for balance, dealer total, and player total
        renderText(renderer, "Balance:", 0, 0, 20, {255, 255, 255, 255});
        renderText(renderer, buffer, 100, 0, 20, {255, 255, 255, 255});
        renderText(renderer, "DealerTotal", 0, 250, 20, {255, 255, 255, 255});
        renderText(renderer, dtot, 50, 280, 30, {255, 255, 255, 255});
        renderText(renderer, "Player Total", 0, 500, 20, {255, 255, 255, 255});
        renderText(renderer, ptot, 50, 530, 30, {255, 255, 255, 255});
        // Add additional conditions for other game states as needed
    }
    else if (game_state == 5)
    {
        // Render the background and the dealer won screen
        SDL_RenderCopy(renderer, background, &bg_src_rect, &bg_des_rect);
        SDL_RenderCopy(renderer, dealerWon, &bg_src_rect, &bg_des_rect);
        // Render texts for balance, dealer total, and player total
        renderText(renderer, "Balance:", 0, 0, 20, {255, 255, 255, 255});
        renderText(renderer, buffer, 100, 0, 20, {255, 255, 255, 255});
        renderText(renderer, "DealerTotal", 0, 250, 20, {255, 255, 255, 255});
        renderText(renderer, dtot, 50, 280, 30, {255, 255, 255, 255});
        renderText(renderer, "Player Total", 0, 500, 20, {255, 255, 255, 255});
        renderText(renderer, ptot, 50, 530, 30, {255, 255, 255, 255});
    }
    else
    {
        // Render the background, cards, and controls
        SDL_RenderCopy(renderer, background, &bg_src_rect, &bg_des_rect);
        SDL_RenderCopy(renderer, cards, &flipped_card_src_rect, &flipped_card_des_rect);
        SDL_RenderCopy(renderer, controls, &bg_src_rect, &bg_des_rect);
        // Render texts for balance, dealer total, and player total
        renderText(renderer, "Balance:", 0, 0, 20, {255, 255, 255, 255});
        renderText(renderer, buffer, 100, 0, 20, {255, 255, 255, 255});
        renderText(renderer, "DealerTotal", 0, 250, 20, {255, 255, 255, 255});
        renderText(renderer, dtot, 50, 280, 30, {255, 255, 255, 255});
        renderText(renderer, "Player Total", 0, 500, 20, {255, 255, 255, 255});
        renderText(renderer, ptot, 50, 530, 30, {255, 255, 255, 255});
    }
    // Render player and dealer cards if in the appropriate game states
    if (game_state == 3 || game_state == 4 || game_state == 5)
    {
        // Render player cards
        for (int i = 0; i < playerCardCount; i++)
        {
            SDL_Rect srcRect = {playerCards[i].num * 79, playerCards[i].suit * 123, 79, 123};
            int xdist = 400 - ((playerCardCount * 79) / 2);
            SDL_Rect desRect = {xdist + (i * 79), 450, 79, 123};
            SDL_RenderCopy(renderer, cards, &srcRect, &desRect);
        }
        // Render dealer cards
        for (int i = 0; i < dealerCardCount; i++)
        {
            // SDL_Rect srcRect = {2*79, 4*123, 79, 123};
            SDL_Rect srcRect = {dealerCards[i].num * 79, dealerCards[i].suit * 123, 79, 123};
            int xdist = 400 - ((dealerCardCount * 79) / 2);
            SDL_Rect desRect = {xdist + (i * 79), 200, 79, 123};
            if (i == 1 && cardHidden)
            {
                SDL_RenderCopy(renderer, cards, &flipped_card_src_rect, &desRect);
            }
            else
            {
                SDL_RenderCopy(renderer, cards, &srcRect, &desRect);
            }
        }
    }

    SDL_RenderPresent(renderer);
    // SDL_RenderCopy(renderer, background, &bg_src_rect, &bg_des_rect);
}


// Function to destroy SDL window and renderer
///////////////////////////////////////////////////////////////////////////////
void destroy_window(void)
{
    SDL_DestroyTexture(startScreen);
    SDL_DestroyTexture(balanceScreen);
    SDL_DestroyTexture(betScreen);
    SDL_DestroyTexture(background);
    SDL_DestroyTexture(cards);
    IMG_Quit();
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}


// Main function
///////////////////////////////////////////////////////////////////////////////
int main(int argc, char *args[])
{
    game_is_running = initialize_window();

    setup();
    Screen();

    while (game_is_running)
    {
        process_input();
        update();
        render();
    }

    destroy_window();

    return 0;
}
