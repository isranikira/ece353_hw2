/**
 * @file hw02.c
 * @author KIRA ISRANI israni2@wisc.edu & Breckin Younker byounker@wisc.edu
 * @brief
 * @version 0.1
 * @date 2023-09-16
 *
 * @copyright Copyright (c) 2023
 *
 */
#include "main.h"
#include "hw02.h"
#include "hw02-lcd-staff.h"
#include "hw02-images.h"

/*****************************************************************************/
/*  Global Variables                                                         */
/*****************************************************************************/

uint8_t player_number = 0; // 0 for no player selected yet, 1 for player 1, 2 for player 2;
uint8 player_symbol = 0;   // 0 for O, 1 for X
bool setup_mode = true;

extern cyhal_uart_t cy_retarget_io_uart_obj;
uint8_t remote_rx_message[80];

char HW02_DESCRIPTION[] = "ECE353: HW02 KIRA ISRANI, Breckin Younker";

bool o_goes_first = false; // determines who goes first
bool curr_board_going;     // keeps track of if it is O or X turn
bool winner = false;       // keeps track of if the game has a result or not

uint8_t board_state[3][3] = {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}}; // 0 = not taken, 1 = O, 2 = X

// enum for how to move the square selector
typedef enum
{
    MOVE_UP,
    MOVE_DOWN,
    MOVE_RIGHT,
    MOVE_LEFT,
    MOVE_NONE
} move_direction;

bool ack_received = false;

// This hold when a button has been pressed
typedef enum
{
    ALERT_NONE, /* There is No Event Present*/
    ALERT_BUTTON
} hw02_timer_alerts_t;

typedef enum
{
    P1_O,
    P1_X,
    LT, 
    CT, 
    RT,
    LC, 
    CC, 
    RC,
    LB, 
    CB, 
    RB,
    NO_MESSAGE
} MESSAGE_IN;

MESSAGE_IN currMES = NO_MESSAGE;

volatile hw02_timer_alerts_t ALERT_SW1 = ALERT_NONE;
volatile hw02_timer_alerts_t ALERT_SW2 = ALERT_NONE;

volatile int fg_color; // square foreground color
volatile int bg_color; // square background color
volatile joystick_position_t curr_joystick_pos;
volatile uint32_t curr_col = 1; // where the user is on the board
volatile uint32_t curr_row = 1;

volatile uint32_t old_col_board = 1; //used to update when moving off a square
volatile uint32_t old_row_board = 1;  // and when other board selects a square
volatile uint32_t curr_col_board; //needs to be global gets changed from other board as well
volatile uint32_t curr_row_board;

#define TICKS_MS_100 (10000000)

move_direction move_dir = MOVE_NONE;
cyhal_timer_t timer_obj;
cyhal_timer_cfg_t timer_cfg;

void Handler_Timer(void *handler_arg, cyhal_timer_event_t event);
/*****************************************************************************/
/*  HW02 Functions                                                           */
/*****************************************************************************/

/**
 * @brief
 * Initializes the PSoC6 Peripherals used for HW01
 */
void hw02_peripheral_init(void)
{
    /* Initialize the pushbuttons */
    push_buttons_init();

    /* Initialize the LCD */
    ece353_enable_lcd();

    /* Initialize the joystick*/
    joystick_init();

    /* Initialize the remote UART */
    remote_uart_init();
    remote_uart_enable_interrupts();

    /* Initialize Timer to generate interrupts every 100mS*/
    timer_init(&timer_obj, &timer_cfg, TICKS_MS_100, Handler_Timer);
}

/**
 * @brief
 * Changes draw varavles based on if the
 * square is filled and determines what
 * square to draw base on the joystick
 */

/**
 * @brief
 * Interrupt Handler for the timer, checks the buttons and joystick every 10MS
 */
void Handler_Timer(void *handler_arg, cyhal_timer_event_t event)
{
    uint32_t reg_val = PORT_BUTTONS->IN;
    static bool prev_sw1_pressed = false;
    static bool prev_sw2_pressed = false;
    static joystick_position_t old_pos = JOYSTICK_POS_CENTER;
    joystick_position_t curr_pos = joystick_get_pos();

    if (old_pos == JOYSTICK_POS_CENTER && curr_pos != JOYSTICK_POS_CENTER)
    {
        old_pos = curr_pos;
        if (curr_pos == JOYSTICK_POS_DOWN)
        {
            move_dir = MOVE_DOWN;
        }
        else if (curr_pos == JOYSTICK_POS_UP)
        {
            move_dir = MOVE_UP;
        }
        else if (curr_pos == JOYSTICK_POS_LEFT)
        {
            move_dir = MOVE_LEFT;
        }
        else if (curr_pos == JOYSTICK_POS_RIGHT)
        {
            move_dir = MOVE_RIGHT;
        }
        else
        {
            move_dir = MOVE_NONE;
        }
    }
    else
    {
        move_dir = MOVE_NONE;
        old_pos = curr_pos;
    }

    // check if button one is pressed
    if ((reg_val & SW1_MASK) == 0)
    {
        if (!prev_sw1_pressed)
        {
            printf("button 1 pressed\r\n");
            prev_sw1_pressed = true;
            ALERT_SW1 = ALERT_BUTTON; // set button alert
        }
    }
    else
    {
        prev_sw1_pressed = false;
    }

    // Check if button 2 is pressed
    if ((reg_val & SW2_MASK) == 0)
    {
        if (!prev_sw2_pressed)
        {
            printf("button 2 pressed\r\n");
            prev_sw2_pressed = true;
            ALERT_SW2 = ALERT_BUTTON; // set button alert
        }
    }
    else
    {
        prev_sw2_pressed = false;
    }
}

/**
 * @brief
 * Check to see if the game is over
 * it checks for O or X winner or a full board tie
 */
void check_board()
{

    // check rows
    for (int i = 0; i < 3; i++)
    {
        if (board_state[i][0] == board_state[i][1] && board_state[i][0] == board_state[i][2])
        {
            if (board_state[i][0] == 1)
            {
                lcd_O_wins();
                winner = true;
                break;
            }
            if (board_state[i][0] == 2)
            {
                lcd_X_wins();
                winner = true;
                break;
            }
        }
    }

    // Check cols
    for (int i = 0; i < 3; i++)
    {
        if (board_state[0][i] == board_state[1][i] && board_state[2][i] == board_state[0][i])
        {
            if (board_state[0][i] == 1)
            {
                lcd_O_wins();
                winner = true;
                break;
            }
            if (board_state[0][i] == 2)
            {
                lcd_X_wins();
                winner = true;
                break;
            }
        }
    }

    // check diagonal 1
    if (board_state[0][2] == board_state[1][1] && board_state[2][0] == board_state[0][2])
    {
        if (board_state[0][2] == 1)
        {
            lcd_O_wins();
            winner = true;
        }
        if (board_state[0][2] == 2)
        {
            lcd_X_wins();
            winner = true;
        }
    }

    // check diagonal 2
    if (board_state[0][0] == board_state[1][1] && board_state[2][2] == board_state[0][0])
    {
        if (board_state[2][2] == 1)
        {
            lcd_O_wins();
            winner = true;
        }
        if (board_state[2][2] == 2)
        {
            lcd_X_wins();
            winner = true;
        }
    }

    // check for tie
    bool game_done = true;
    for (int i = 0; i < 3; i++)
    {
        for (int j = 0; j < 3; j++)
        {
            if (board_state[i][j] == 0)
            {
                game_done = false;
            }
        }
    }
    if (game_done)
    {
        winner = true;
        lcd_tie();
    }

    // This will flip if the game is over
    if (winner)
    {

        // Fix me
    }
}

void is_player_selected()
{
    if (ALERT_SW2 && player_number == 0)
    {
        ALERT_SW2 = false;
        remote_uart_tx_data_async("5A\r");
        printf("p1 tried to assign itself p1 and sent sig to p2\r\n");
    }

    // check if recved data from other board
    if (ALERT_UART_RX)
    {
        memset(remote_rx_message, 0, 80);
        ALERT_UART_RX = false;

        remote_uart_rx_data_async(remote_rx_message, 80);

        // if it is new data
        if (remote_rx_message[0] == '5' && remote_rx_message[1] == 'A')
        {

            player_number = 2;                 // make player 2
            remote_uart_tx_data_async("F0\r"); // send other board acknowledgement
            printf("player 2 got assigned sent signal to p1\r\n");
            ack_received = true;
        }
        else if (remote_rx_message[0] == 'F' && remote_rx_message[1] == '0')
        {
            player_number = 1;
            printf("player 1 recieved ack siganlal\r\n");
            ack_received = true;
        }
    }
}

void get_UART_MESSAGE()
{
    if (ALERT_UART_RX)
    {
        memset(remote_rx_message, 0, 80);
        ALERT_UART_RX = false;

        remote_uart_rx_data_async(remote_rx_message, 80);

        if (remote_rx_message[0] == '5' && remote_rx_message[1] == '8')
        {
            currMES = P1_O;
            return;
        }

        if (remote_rx_message[0] == '4' && remote_rx_message[1] == 'F')
        {
            currMES = P1_X;
            return;
        }

        if (remote_rx_message[0] == '0' && remote_rx_message[1] == '0')
        {
            old_col_board = 0;
            old_row_board = 0;
            curr_col_board = LEFT_COL;
            curr_row_board = UPPER_ROW;
            currMES = LT;
            if (player_symbol == 0){
                board_state[0][0] = 1;
            } else{
                board_state[0][0] = 2;
            }
            return;
        }

        if (remote_rx_message[0] == '1' && remote_rx_message[1] == '0')
        {
            old_col_board = 1;
            old_row_board = 0;
            curr_col_board = CENTER_COL;
            curr_row_board = UPPER_ROW;
            currMES = CT;
            if (player_symbol == 0){
                board_state[1][0] = 1;
            } else{
                board_state[1][0] = 2;
            }
            return;
        }
        if (remote_rx_message[0] == '2' && remote_rx_message[1] == '0')
        {
            curr_col_board = RIGHT_COL;
            curr_row_board = UPPER_ROW;
            old_col_board = 2;
            old_row_board = 0;
            currMES = RT;
            if (player_symbol == 0){
                board_state[2][0] = 1;
            } else{
                board_state[2][0] = 2;
            }
            return;
        }
        if (remote_rx_message[0] == '0' && remote_rx_message[1] == '1')
        {
            curr_col_board = LEFT_COL;
            curr_row_board = CENTER_ROW;
            old_col_board = 0;
            old_row_board = 1;
            currMES = LC;
            if (player_symbol == 0){
                board_state[0][1] = 1;
            } else{
                board_state[0][1] = 2;
            }
            return;
        }
        if (remote_rx_message[0] == '1' && remote_rx_message[1] == '1')
        {
            curr_col_board = CENTER_COL;
            curr_row_board = CENTER_ROW;
            old_col_board = 1;
            old_row_board = 1;
            currMES = CC;
            if (player_symbol == 0){
                board_state[1][1] = 1;
            } else{
                board_state[1][1] = 2;
            }
            return;
        }
        if (remote_rx_message[0] == '2' && remote_rx_message[1] == '1')
        {
            curr_col_board = RIGHT_COL;
            curr_row_board = CENTER_ROW;
            old_col_board = 2;
            old_row_board = 1;
            currMES = RC;
            if (player_symbol == 0){
                board_state[2][1] = 1;
            } else{
                board_state[2][1] = 2;
            }
            return;
        }
        if (remote_rx_message[0] == '0' && remote_rx_message[1] == '2')
        {
            curr_col_board = LEFT_COL;
            curr_row_board = LOWER_ROW;
            old_col_board = 0;
            old_row_board = 2;
            currMES = LB;
            if (player_symbol == 0){
                board_state[0][2] = 1;
            } else{
                board_state[0][2] = 2;
            }
            return;
        }
        if (remote_rx_message[0] == '1' && remote_rx_message[1] == '2')
        {
            curr_col_board = CENTER_COL;
            curr_row_board = LOWER_ROW;
            old_col_board = 1;
            old_row_board = 2;
            currMES = CB;
            if (player_symbol == 0){
                board_state[1][2] = 1;
            } else{
                board_state[1][2] = 2;
            }
            return;
        }
        if (remote_rx_message[0] == '2' && remote_rx_message[1] == '2')
        {
            curr_col_board = RIGHT_COL;
            curr_row_board = LOWER_ROW;
            old_col_board = 2;
            old_row_board = 2;
            currMES = RB;
            if (player_symbol == 0){
                board_state[2][2] = 1;
            } else{
                board_state[2][2] = 2;
            }
            return;
        }
    }
    currMES = NO_MESSAGE;
}

void curr_position()
{
    // changes slected square and wraps around if needed
    if (move_dir == MOVE_DOWN)
    {
        curr_row = (curr_row + 1) % 3;
    }
    else if (move_dir == MOVE_UP)
    {
        curr_row = (curr_row + 2) % 3;
    }
    else if (move_dir == MOVE_LEFT)
    {
        curr_col = (curr_col + 2) % 3;
    }
    else if (move_dir == MOVE_RIGHT)
    {
        curr_col = (curr_col + 1) % 3;
    }
    move_dir = MOVE_NONE;
}

void draw_board()
{
    printf("Attempt to draw board\n\r");
    // static uint32_t curr_col_board;
    // static uint32_t curr_row_board;
    // static uint32_t past_col_board = 1;
    // static uint32_t past_row_board = 1;
    uint16_t size_rec = 36;
    uint32_t fg_color;
    uint32_t bg_color;

    //before updating the curr_col_board draw what square you are leaving
     //assign colors based on wether assigned or not
     //TODO: colors neeed to change just using for testing
    if (board_state[old_row_board][old_col_board] == 0)
    {
        fg_color = LCD_COLOR_BLACK;
        bg_color = LCD_COLOR_BLACK;
    }
    else if(board_state[old_row_board][old_col_board] == 1)
    {
        fg_color = LCD_COLOR_RED;
        bg_color = LCD_COLOR_BLACK;
    }
    else if(board_state[old_row_board][old_col_board] == 2)
    {
        fg_color = LCD_COLOR_BLUE;
        bg_color = LCD_COLOR_BLACK;
    }
    else //should never enter here
    {
        fg_color = LCD_COLOR_WHITE;
        bg_color = LCD_COLOR_BLACK;
    }
    //draw bg color of square you are moving out of
    lcd_draw_rectangle_centered(curr_col_board, curr_row_board, 36, 36, bg_color);
    //if the board state has a taken state draw it
    if (board_state[old_row_board][old_col_board] == 1)
    {
        lcd_draw_image(curr_col_board, curr_row_board, O_WIDTH, O_HEIGHT, Bitmaps_O, fg_color, bg_color);
    }
    else
    {
        lcd_draw_image(curr_col_board, curr_row_board, X_WIDTH, X_HEIGHT, Bitmaps_X, fg_color, bg_color);
    }

    if (curr_col == 0)
    {
        curr_col_board = LEFT_COL;
    }
    else if (curr_col == 1)
    {
        curr_col_board = CENTER_COL;
    }
    else if (curr_col == 2)
    {
        curr_col_board = RIGHT_COL;
    }

    if (curr_row == 0)
    {
        curr_row_board = UPPER_ROW;
    }
    else if (curr_row == 1)
    {
        curr_row_board = CENTER_ROW;
    }
    else if (curr_row == 2)
    {
        curr_row_board = LOWER_ROW;
    }

    //assign fg and bg colors depending if square is free or not
    if (board_state[curr_row][curr_col] == 0)
    {
        fg_color = LCD_COLOR_BLACK;
        bg_color = LCD_COLOR_GREEN;
    }
    else
    {
        fg_color = LCD_COLOR_BLACK;
        bg_color = LCD_COLOR_RED;
    }

    lcd_draw_rectangle_centered(curr_col_board, curr_row_board, 36, 36, bg_color);

    if (player_symbol == 0)
    {
        lcd_draw_image(curr_col_board, curr_row_board, O_WIDTH, O_HEIGHT, Bitmaps_O, fg_color, bg_color);
    }
    else
    {
        lcd_draw_image(curr_col_board, curr_row_board, X_WIDTH, X_HEIGHT, Bitmaps_X, fg_color, bg_color);
    }

    old_row_board = curr_row;
    old_col_board = curr_col;

    // if (past_col_board == 0)
    // {
    //     curr_col_board = LEFT_COL;
    // }
    // else if (past_col_board == 1)
    // {
    //     curr_col_board = CENTER_COL;
    // }
    // else if (past_col_board == 2)
    // {
    //     curr_col_board = RIGHT_COL;
    // }

    // if (past_row_board == 0)
    // {
    //     curr_row_board = UPPER_ROW;
    // }
    // else if (past_row_board == 1)
    // {
    //     curr_row_board = CENTER_ROW;
    // }
    // else if (past_row_board == 2)
    // {
    //     curr_row_board = LOWER_ROW;
    // }

    // //assign colors based on wether assigned or not
    // if (board_state[curr_row][curr_col] == 0)
    // {
    //     fg_color = LCD_COLOR_BLACK;
    //     bg_color = LCD_COLOR_GREEN;
    // }
    // else
    // {
    //     fg_color = LCD_COLOR_BLACK;
    //     bg_color = LCD_COLOR_RED;
    // }
    // lcd_draw_rectangle_centered(curr_col_board, curr_row_board, 36, 36, LCD_COLOR_BLACK);

    // past_col_board = curr_col;
    // past_row_board = curr_row;
}

/**
 * @brief
 * Implements the main application for HW02
 */
void hw02_main_app(void)
{
    bool your_turn = false;
    lcd_select_player1();
    memset(remote_rx_message, 0, 80);

    // while loop (check if sw2 was pressed or uart recieved (5A)) - determine who player 1 is
    //  check if sw2 is presed or the uart is recieved
    // if sw2 is pressed handler will have a while loop waiting for acknowledgment
    // if not pressed but recieves p1 selection byte send acknowledgement
    // set player 1 to curr board going
    while (!ack_received)
    {
        is_player_selected();
    }
    lcd_draw_rectangle_centered(SCREEN_CENTER_COL, UPPER_HORIZONTAL_LINE_Y, 300, 100, LCD_COLOR_BLACK);
    tic_tac_toe_draw_grid();

    if (player_number == 1)
    {
        bool wait = true;

        while (wait)
        {
            if (player_symbol == 0) 
            {
                lcd_draw_image(CENTER_COL, CENTER_ROW, O_WIDTH, O_HEIGHT, Bitmaps_O, LCD_COLOR_GREEN, LCD_COLOR_BLACK);
            }
            else
            {
                lcd_draw_image(CENTER_COL, CENTER_ROW, X_WIDTH, X_HEIGHT, Bitmaps_X, LCD_COLOR_GREEN, LCD_COLOR_BLACK);
            }

            if (ALERT_SW1)
            {
                player_symbol = !player_symbol;
                ALERT_SW1 = ALERT_NONE;
            }

            if (ALERT_SW2 && player_symbol == 0)
            {
                wait = false;
                remote_uart_tx_data_async("58\r");
                ALERT_SW2 = ALERT_NONE;
                your_turn = true;
            }

            if (ALERT_SW2 && player_symbol == 1)
            {
                wait = false;
                remote_uart_tx_data_async("4F\r");
                ALERT_SW2 = ALERT_NONE;
                your_turn = true;
            }
        }
    }
    else
    {
        bool wait = true;
        lcd_wait_for_other_player();
        while (wait)
        {
            get_UART_MESSAGE();
            if (currMES == P1_O)
            {
                player_symbol = 1;
                wait = false;
                remote_uart_tx_data_async("F0\r");
                lcd_clear_other_player();
            }
            else if (currMES == P1_X)
            {
                player_symbol = 1;
                wait = false;
                remote_uart_tx_data_async("F0\r");
                lcd_clear_other_player();
            }
        }
    }

    while (1)
    {
        while (!your_turn)
        {
            lcd_wait_for_other_player();
            get_UART_MESSAGE();
            if (currMES != NO_MESSAGE)
            {
                your_turn = true;
                lcd_clear_other_player();
                draw_board();
            }
        }

        curr_position();
        draw_board();

        //wait while loop if no one is moving and the square is not being selected
        while (move_dir == MOVE_NONE && !ALERT_SW1)
        {
        }

        
        if (ALERT_SW1)
        {
            ALERT_SW1 = false;
            your_turn = false;

            if (board_state[curr_row][curr_col] == 0){
                if (player_symbol == 0){
                    board_state[curr_row][curr_col] = 1;
                } else {
                    board_state[curr_row][curr_col] = 2;
                }

                if (curr_row == 0 && curr_col == 0){
                    remote_uart_tx_data_async("00\r");
                }
                if (curr_row == 1 && curr_col == 0){
                    remote_uart_tx_data_async("10\r");
                }
                if (curr_row == 2 && curr_col == 0){
                    remote_uart_tx_data_async("20\r");
                }
                if (curr_row == 0 && curr_col == 1){
                    remote_uart_tx_data_async("01\r");
                }
                if (curr_row == 1 && curr_col == 1){
                    remote_uart_tx_data_async("11\r");
                }
                if (curr_row == 2 && curr_col == 1){
                    remote_uart_tx_data_async("21\r");
                }
                if (curr_row == 0 && curr_col == 2){
                    remote_uart_tx_data_async("02\r");
                }
                if (curr_row == 1 && curr_col == 2){
                    remote_uart_tx_data_async("12\r");
                }
                if (curr_row == 2 && curr_col == 2){
                    remote_uart_tx_data_async("22\r");
                }
            }
            fg_color = LCD_COLOR_GREEN;
            bg_color = LCD_COLOR_BLACK;
            draw_board();
            fg_color = LCD_COLOR_BLACK;
            bg_color = LCD_COLOR_GREEN;
        }

        check_board();
        //need to reset vars and start over
    }
}

//bugs:
// wont draw squares to the board
// fills board array with its own number
// doesnt restart when there is a winner
// if you select a square that has already been chosen boards go into locked state


