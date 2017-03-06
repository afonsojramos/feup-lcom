#ifndef __VIDEO_GR_H
#define __VIDEO_GR_H

#include "settings.h"

/*!
 * @struct Paddle
 * @brief The paddle struct.
 * The paddle object is the white rectangle controllable by the player.
 * Only moves horizontally and serves the purpose of avoiding losing the ball.
 * Only one single paddle is initialized throughout the course of the game.
 */
typedef struct{
	unsigned short x;		//! The paddle's x position.
	unsigned short y;		//! The paddle's y position.
	unsigned short speed;	//! The paddle's x speed, measured in pixels.
	char** sprite;			//! The paddle's associated sprite.
} Paddle;

/*!
 * @struct Ball
 * @brief The ball struct.
 * The ball collides with bricks in order to score points, yet it isn't controllable by the player.
 * The player can, however, control its trajectory by hitting it with the paddle.
 * Multiple objects may be generated from this struct, provided that the player activated the 'Extra Ball' power-up.
 */
typedef struct{
	unsigned short x;			//! The ball's x position.
	unsigned short y;			//! The ball's y position.
	unsigned short speedx;		//! The ball's x speed, measured in pixels.
	unsigned short speedy;		//! The ball's y speed, measured in pixels.
	unsigned short dir;			//! The ball's preserved direction, vital for movement.
	char** sprite;				//! The ball's associated sprite.
} Ball;

/*!
 * @struct Brick
 * @brief The brick struct.
 * Every time the ball collides with a brick, the player's score is incremented based on the current multiplier.
 * The bricks are displayed in six different colours: pink, red, orange, yellow, green and blue.
 * Multiple objects from this struct are generated every game and stored in an array.
 */
typedef struct{
	unsigned short x;		//! The brick's x position.
	unsigned short y;		//! The brick's y position.
	int visible;			//! Whether the brick is a candidate for collision or not. 0 = invisible; 1 = visible.
	char** sprite;			//! The brick's associated sprite.
} Brick;

/*!
 * @struct Cursor
 * @brief The cursor struct.
 * A cursor controllable by the user using the mouse peripheral.
 * When the cursor object, based on its coordinates, collides with a menu option, this option is highlighted which two identical rectangles on the top and bottom of the sprite.
 */
typedef struct{
	unsigned short x;		//! The cursor's x position.
	unsigned short y;		//! The cursor's y position.
	float sensitivity;		//! The cursor's sensitivity which hinders its velocity.
	char** sprite;			//! The cursor's associated sprite.
} Cursor;

/*!
 * @struct Score
 * @brief The score struct.
 * The score is displayed while the game is running as the form of a 3 digit integer.
 * It is incremented based on the current multiplier and whether the ball has collided with a brick.
 */
typedef struct{
	int value;			//! The score's value.
	char** digit0;		//! The rightmost digit's associated sprite.
	char** digit1;		//! The middle digit's associated sprite.
	char** digit2;		//! The leftmost digit's associated sprite.
} Score;

/*!
 * @struct Timer
 * @brief The timer struct.
 * The timer counts the game's elapsed time since its start.
 * It is updated every 60 timer interruptions (1 second), counting for a maximum of one hundred minutes.
 * @note We opted not to use the RTC on this struct, because we hadn't implemented it at the time. Either way, this implementation ended up being more practical.
 */
typedef struct{
	int seconds;		//! The timer's seconds.
	int minutes;		//! The timer's minutes.
	char** digit0;		//! The second's rightmost digit's associated sprite.
	char** digit1;		//! The second's leftmost digit's associated sprite.
	char** digit2;		//! The minute's rightmost digit's associated sprite.
	char** digit3;		//! The minute's leftmost digit's associated sprite.
} Timer;

/*!
 * @struct Powerup
 * @brief The powerup struct.
 * Three powerups were implemented: 'x2 Multiplier', which doubles the current multiplier; 'x4 Multiplier', which quadruplicates the current multiplier; 'Extra Ball', which adds an extra playable ball.
 * Powerups have a chance to drop after a ball has collided with a brick and stay active indefinitely.
 */
typedef struct{
	unsigned short x;			//! The powerup's x position.
	unsigned short y;			//! The powerup's y position.
	unsigned short dropping;	//! Whether the powerup is currently dropping.
	unsigned short active;		//! Whether the powerup is currently active.
	char** sprite;				//! The powerup's associated sprite.
} Powerup;

/*!
 * @struct Highlight
 * @brief The highlight struct.
 * Every time the mouse/keyboard hovers an option, two identical rectangles are displayed on the top and bottom of the menu option.
 */
typedef struct{
	unsigned short h1;		//! Mouse is hovering the 'Play' button.
	unsigned short h2;		//! Mouse is hovering the 'Highscores' button.
	unsigned short h3;		//! Mouse is hovering the 'Exit' button.
	unsigned short h4;		//! Mouse is hovering the 'Return To Main Menu' button.
	unsigned short h5;		//! Keyboard selected the first letter on the 'New Highscore' menu.
	unsigned short h6;		//! Keyboard selected the second letter on the 'New Highscore' menu.
	unsigned short h7;		//! Keyboard selected the third letter on the 'New Highscore' menu.
	unsigned short h8;		//! Mouse is hovering the 'Submit' button.
} Highlight;

/*!
 * @struct Highscore
 * @brief The highscore struct.
 * A highscore is displayed when the user has reached a higher score than the top 8 players.
 * A highscore contains 3 letters, the score and the date when the highscore was set.
 * Every highscore is stored on an array and furthermore, extracted from and saved to a file.
 */
typedef struct{
	char alpha1;			//! The first char of the player's initials.
	char alpha2;			//! The second char of the player's initials.
	char alpha3;			//! The third char of the player's initials.
	char** alpha1sprite;	//! The first char's associated sprite.
	char** alpha2sprite;	//! The second char's associated sprite.
	char** alpha3sprite;	//! The third char's associated sprite.
	int value;				//! The highscore, an integer.
	char** digit0sprite;	//! The score's rightmost digit's associated sprite.
	char** digit1sprite;	//! The score's middle digit's associated sprite.
	char** digit2sprite;	//! The score's leftmost digit's associated sprite.
	int date;				//! The date when the highscore was set.
	char** day0sprite;		//! The day's rightmost digit's associated sprite.
	char** day1sprite;		//! The day's leftmost digit's associated sprite.
	char** month0sprite;	//! The month's rightmost digit's associated sprite.
	char** month1sprite;	//! The month's leftmost digit's associated sprite.
} Highscore;

/*!
 * @struct FontCycle
 * @brief The fontcycle struct.
 * Contains 3 iterators that cycle through the alphabet.
 * Allows for a 'nickname' selection on the 'New Highscore' menu.
 */

typedef struct{
	unsigned int iterator1;		//The leftmost char iterator.
	unsigned int iterator2;		//The middle char iterator.
	unsigned int iterator3;		//The last char iterator.
} FontCycle;

/*!
 * @struct Clock
 * @brief The clock struct.
 * This is the computer's time, displayed on the main menu and on each highscore.
 * The clock stores the hour, minute, second, day, month and year and is updated every second based on the RTC.
 */

typedef struct{
	unsigned long hour;		//The current hour.
	char** hourDigitL;		//The leftmost hour digit associated sprite.
	char** hourDigitR;		//The rightmost hour digit associated sprite.
	unsigned long minute;	//The current minute.
	char** minuteDigitL;	//The leftmost minute digit associated sprite.
	char** minuteDigitR;	//The rightmost minute digit associated sprite.
	unsigned long second;	//The current second.
	char** secondDigitL;	//The leftmost second digit associated sprite.
	char** secondDigitR;	//The rightmost second digit associated sprite.
	unsigned long day;		//The current day.
	char** dayDigitL;		//The leftmost day digit associated sprite.
	char** dayDigitR;		//The rightmost day digit associated sprite.
	unsigned long month;	//The current month.
	char** monthDigitL;		//The leftmost month digit associated sprite.
	char** monthDigitR;		//The rightmost month digit associated sprite.
	unsigned long year;		//The current year.
	char** yearDigitLM;		//The leftmost year digit associated sprite.
	char** yearDigitL;		//The second left year digit associated sprite.
	char** yearDigitR;		//The second right year digit associated sprite.
	char** yearDigitRM;		//The rightmost year digit associated sprite.
} Clock;

/*!
 * @struct Flow
 * @brief The flow struct.
 * This allows for manipulation and quick access of certain aspects and variables of the game at given points.
 */
typedef struct{
	int multiplier;			//The current score multiplier.
	short powDropRate;		//The current powerup drop rate.
	short powDropSpeed;		//The current powerup drop speed.
} Flow;

/*!
 * @struct Render
 * @brief The render struct.
 * This struct decides what will be rendered each frame, so that excessive weight is lifted from the video memory, instead of rendering everything each frame.
 */
typedef struct{
	short ball;				//If 1, render the ball.
	short ball2;			//If 1, render the extra ball.
	short paddle;			//If 1, render the paddle.
	short bricks;			//If 1, render the bricks.
	short boundaries;		//If 1, render the grey game limits.
	short cursor;			//If 1, render the cursor.
	short gui;				//If 1, renders the score and timer.
	short logos;			//If 1, renders the main menu logos.
	short powerup;			//If 1, renders the powerups.
	short highscores;		//If 1, renders the highscores.
	short highlights;		//If 1, renders the highlights.
	short submit;			//If 1, renders the char selection and submit button.
	short clock;			//If 1, renders the clock.
} Render;

static Paddle paddle;
static Ball ball;
static Ball ball2;
static Brick brick;
static Cursor cursor;
static Score score;
static Timer timer;
static Powerup powerup;
static Highlight highlight;
static Highscore highscore;
static FontCycle fontcycle;
static Clock clock;
static Flow flow;
static Render render;

static Brick allBricks[MAX_BRICKS];
static Powerup allPowerups[5];
Highscore allHighscores[8];
static char alphabet[26];
static int brickSize = 0;
static int ballNumber = 1;

/** @defgroup video_gr video_gr
 * @{
 *
 * Functions for outputing data to screen in graphics mode
 */

/**
 * @brief Initializes the video module in graphics mode
 * 
 * Uses the VBE INT 0x10 interface to set the desired
 *  graphics mode, maps VRAM to the process' address space and
 *  initializes static global variables with the resolution of the screen, 
 *  and the number of colors
 * 
 * @param mode 16-bit VBE mode to set
 * @return Virtual address VRAM was mapped to. NULL, upon failure.
 */
void *vg_init(unsigned short mode);

 /**
 * @brief Returns to default Minix 3 text mode (0x03: 25 x 80, 16 colors)
 * 
 * @return 0 upon success, non-zero upon failure
 */
int vg_exit(void);

/*!
 * @brief Paints a pixel in a certain position with a given colour.
 * @param x X coordinate of the pixel.
 * @param y Y coordinate of the pixel.
 * @param color Color the pixel should be painted with, ranging from 0 (black) to 63 (white).
 */
int switchPixel(unsigned short x, unsigned short y, unsigned long color);

/*!
 * @brief Draws a rectangle on screen.
 * This uses the switchPixel function and is how the game boundaries, as well as the menu hover highlighting are accomplished.
 * @param x Starting x coordinate of the rectangle (top left pixel).
 * @param y Starting y coordinate of the rectangle (top left pixel).
 * @param xSize Width of the rectangle, from its starting x coordinate.
 * @param ySize Height of the rectangle, from its starting y coordinate.
 * @param color Color of the rectangle.
 */
int drawRectangle(unsigned short x, unsigned short y, unsigned short xSize, unsigned short ySize, unsigned long color);

/*!
 * @brief Displays a XPM image on the screen.
 * @note We opted to use XPM instead of BMP, because our graphics were simple enough that the extra work necessary to implement bitmap images wouldn't pay off.
 * @param xi Starting x coordinate (top left pixel).
 * @param yi Starting y coordinate (top left pixel).
 * @param xpm[] Sprite to render, declared on sprites.h/ stableSprites.h.
 */
int displayXPM(unsigned short xi, unsigned short yi, char *xpm[]);

/*!
 * @brief Clears a XPM image from the screen.
 * This function simply paints the selected XPM's pixels black, lifting weight off of the video memory.
 * @param xi Starting x coordinate (top left pixel).
 * @param yi Starting y coordinate (top left pixel).
 * @param xpm[] Sprite to clear, declared on sprites.h/ stableSprites.h.
 */
void clearSprite(unsigned short xi, unsigned short yi, char *xpm[]);

/*!
 * @brief Switches one powerup's dropping status to active.
 * This function is called whenever there's ball/brick collision and there's a switch if, by chance, the player beat the odds.
 * @param x Drops powerup in x coordinate.
 * @param y Drops powerup in y coordinate.
 */
void dropPowerup(unsigned short x, unsigned short y);

/*!
 * @brief Activates a powerup's effect.
 * After paddle/powerup collision criteria is met, the respective powerup is activated.
 * @param index Position of the powerup in the allPowerups array.
 */
void activateEffect(int index);

/*!
 * @brief Tests whether the player has missed the main or extra ball and it has collided with the window borders.
 */
int testBallCol();

/*!
 * @brief Tests collisions between the main ball and bricks.
 * The collision accomplished by taking into account the relative positions of the ball and every brick at a given moment.
 * If there is a collision, the brick is removed from the array that scores every brick, thus making it disappear when a new frame is rendered (and disabling collisions for it).
 */
int testCollision();

/*!
 * @brief Tests collisions between the powerups and the paddle.
 * The collision accomplished by taking into account the relative positions of the dropping powerup and the paddle at a given moment.
 * If there is a collision, the powerup is activated and disappears.
 */
int testPowerUpCollision();

/*!
 * @brief Tests collisions between the extra ball and bricks.
 * The collision accomplished by taking into account the relative positions of the ball and every brick at a given moment.
 * If there is a collision, the brick is removed from the array that scores every brick, thus making it disappear when a new frame is rendered (and disabling collisions for it).
 */
int testCollision2();

/*!
 * @brief Matches a given character with the respective sprite.
 * @note We felt the urge to create this function, as we were often using individual letters and needed to save a sprite to each.
 * @param c The character to be matched.
 * @return The matched sprite.
 */
char** alphanumericMatch(char c);

/*!
 * @brief Inserts a new highscore.
 * The highscores are sorted, since the insertion takes that into account. This operation also pushes the last highscore out of the array.
 */
void sortHighscore();

/*!
 * @brief Activates the 'New Highscore' window.
 * This condition is only met if the player has managed to get a higher score than the lowest recorded highscore.
 */
void launchHighscore();

/*!
 * @brief Saves highscores to file.
 * Extracts every highscore from the array that scores every single one of them and writes it to file.
 * @note This has proven to be quite inconsistent, so this functionality might be broken.
 */
void saveHighscores();

/*!
 * @brief Tests collisions between the cursor and option menus.
 * This collision allows for mouse hover option highlighting and menu navigating.
 */
int testCursorCol();

/*!
 * @brief Moves the paddle in a given direction.
 * The paddle moves at a fixed speed (pixels), declared in the paddle struct.
 * This function is activated after a mouse button click and 'A'/'D' keypresses.
 * @param direction If 'L', the paddle will move to the left. If 'R', the paddle will move to the right.
 */
void movePaddle(char direction);

/*!
 * @brief Moves the cursor in a given direction.
 * Its movement is relative, so the arguments refer to the coordinates of a second point, forming a straight line from the origin to the new point.
 * This function is called after mouse movement has been detected, and its velocity is affected by the sensitivity parameter in the cursor struct.
 * @param x Where the mouse will land in the x axis.
 * @param y Where the mouse will land in the y axis.
 */
void moveCursor(char x, char y);

/*!
 * @brief Moves the ball while also testing ball/paddle collision.
 * If no collision occurs, the new ball position is set based on its x and y speed and direction.
 * However, if a ball/paddle collision is detected, the ball switches direction (on a 90 degree angle).
 * This function also tests collisions on both the main and extra balls.
 */
void moveBall();

/*!
 * @brief Updates the clock struct.
 * After the RTC values extraction, this function takes the extracted values and updates the clock struct accordingly.
 * @param hour,minute,second,day,month,year Time units to be updated on the clock struct.
 */
void updateTime(unsigned long hour, unsigned long minute, unsigned long second, unsigned long day, unsigned long month, unsigned long year);

/*!
 * @brief Loads the highlight struct.
 * After mouse hovering on a certain menu option, this function is called, which updates the highlight struct.
 * The highlight struct is then used to render white identical rectangles on top of the selected menu option sprite.
 * @param menuOption menuOption to be highlighted (for more information about this matter, please check Doxygen documentation for the highlight struct.)
 */
void highlightOption(unsigned short menuOption);

/*!
 * @brief Iterates through the alphabet in regular or reverse order.
 * Exclusively used in the 'New Highscore' menu where the player is prompted to select a 3 initials for highscore saving.
 * Allows for the user to cycle through the alphabet using the 'W' and 'S' keyboard keys.
 * @param iterator Ranges from 1 to 3, representing which character to iterate.
 * @param direction If 'U', the alphabet is iterated in reverse order. If 'D', the alphabet is iterated in regular order.
 */
void cycleAlphabet(int iterator, char direction);

/*!
 * @brief Loads highscores from text file to memory.
 * Called at the start of each program run.
 * Extracts every highscore from the text file and stores it in an array. The highscores are already sorted by score, in the text file.
 */
void loadHighscores();

/*!
 * @brief Prepares every struct by loading initial values.
 * One of the most important functions of this program. Runs before any frame is rendered.
 * Pre-loads every struct value to ensure proper initialization. This includes: every brick, the paddle, ball, cursor, GUI, powerup, highscore, highlight, alphabet, clock and the flow struct.
 * Every object is initialized at a certain xy position and with its associated sprite (some objects also have speed and integer values that need to be handled).
 * Failing to initialize any value of any struct will most likely result on a black screen and unexpected game behaviour.
 */
void loadSprites();

/*!
 * @brief Returns the number of remaining bricks.
 * @note Because of how we later handled brick management, this function ended up being redundant.
 */
int getBrickSize();

/*!
 * @brief Updates the clock struct.
 * Called every 60 timer interruptions (each second). Updates the clock struct with new time units.
 * Divides every integer (second, minute, hour, ...) into digits and assigns them a sprite.
 * @note We could've used the RTC to handle this, yet it hadn't been implemented at the time and this solution has proven itself to be more practical.
 */
void updateTimer();

/*!
 * @brief Updates the score struct.
 * Called every time there's a ball/brick collision.
 * Score is incremented based on its current multiplier. Works in a very similar fashion to the updateTimer function, as it divides every integer into digits and assigns them a sprite.
 */
void updateScore();

/*!
 * @brief Updates the render struct.
 * Based on the current state of the state machine, the render struct is filled in so that each framer renders only the desired objects.
 * For instance, the ball parameter won't be active in the game menus, yet it obviously will when the player is running the game.
 * This allows for incredible flexibility, such as making the clock spawn at any instance of the program.
 */
void firstRender();

/*!
 * @brief Renders a frame.
 * Based on the render struct, a new frame is generated, every time 60 interruptions have elapsed on the timer.
 * This way of handling frames lifts great weight from the video memory, making the experience as smooth as if double-buffering had been implemented.
 */
void renderFrame();

 /** @} end of video_gr */
 
#endif /* __VIDEO_GR_H */
