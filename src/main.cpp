#include <allegro5/allegro.h>
#include <allegro5/allegro_image.h>
#include <allegro5\allegro_audio.h>
#include <allegro5\allegro_acodec.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>
#include <iostream>

/*///////////////////////////////////////////////////////////////
//DROP BALL VERSUS V1.2                                       //
/////////////////////////////////////////////////////////////////
//By Stewart Valencia                                          //
/////////////////////////////////////////////////////////////////
//Drop Ball Versus is a game based on an "extrememly addictive,//
//high intensity game designed for people everywhere".         //
//                                                             //
//Three modes:                                                 //
//                                                             //
//Single Player Versus - Play against a CPU.                   //
//Versus (Two Player) - Play against your friend.              //
//Endless - Play forever                                       //
//		                                                       //
//Playable characters: Finn, Jake, BMO, Ghost Man              //
///////////////////////////////////////////////////////////////*/

/*GLOBAL VARIABLES*/
//Screen size
const int width = 640;
const int height = 480;

//Current frame of player and CPU sprites
int tarFrame = 0;
int cpuFrame = 0;

//To record scores
int score = 0;
int cpuscore = 0;
int hiscore = 6;

//Keyboard inputs
enum KEYS{A, L, SPACE, LCTRL, RCTRL, S, D, LEFT, RIGHT, DOWN, ENTER};
bool keys[11] = {false, false, false, false, false, false, false, false, false, false, false};

//Types of Game states
enum STATE{TITLE, PLAYING, DROP, GAMEOVER, RESULT, MENU, MENU2, ENDLESS, VERSUS, DROP2, WAIT, WAIT2, GAMEOVER2, ENDDROP};
enum MODE{ONEPLAYER, TWOPLAYER, END};


char *modes[3] = {"Single Player Versus", "Versus", "Endless"};
char *names[4] = {"Ghost Man", "Finn", "Jake", "BMO"};
int options[3] = {PLAYING, VERSUS, ENDLESS};

//Types of sprites
enum PLAYER{GHOST, FINN, JAKE, BMO};

/*Sprite struct - Stores Player and ball images with information*/
struct Sprite
{
	//Player Position
	float x;
	float y;
	//Player Movement
	float velX;
	float velY;
	int dirX;
	int dirY;

	//Player animation
	int maxFrame;
	int curFrame;
	int frameCount;
	int frameDelay;
	int frameWidth;
	int frameHeight;
	int animationColumns;
	int animationDirection;

	//Player Image
	ALLEGRO_BITMAP *image;
};

//Helper Functions
void InitSprites(Sprite &sprite, ALLEGRO_BITMAP *image, int size, int x, int y);
void UpdateSprites(Sprite &sprite, int newFrame);
void UpdateCPU(Sprite &sprite, int cpulimit, Sprite &opBall, int &cpuState, int &state, ALLEGRO_TIMER *clock, ALLEGRO_SAMPLE_INSTANCE **sfx);
void DrawSprites(Sprite &sprite);
void DrawCPU(Sprite &sprite);
void NonDrop(int &state, ALLEGRO_TIMER *clock, int &cpustate, ALLEGRO_SAMPLE_INSTANCE **sfx);
void NonDrop2(int &state, ALLEGRO_TIMER *clock, ALLEGRO_SAMPLE_INSTANCE **sfx);
void NonDrop3(int &state, ALLEGRO_TIMER *clock, ALLEGRO_SAMPLE_INSTANCE **sfx);
void Drop(Sprite &ball, int &state, ALLEGRO_TIMER *clock, int &cpustate,  ALLEGRO_SAMPLE_INSTANCE **sfx);
void Drop2(Sprite &ball, int &state, ALLEGRO_TIMER *clock, ALLEGRO_SAMPLE_INSTANCE **sfx);
void Drop3(Sprite &ball, int &state, ALLEGRO_TIMER *clock, ALLEGRO_SAMPLE_INSTANCE **sfx);
void ChangeState(int &state, int newState, ALLEGRO_TIMER *clock);
void ChangeCPUState(int &state, int newState);

int main(void)
{
	/////////////////////
	/*PROJECT VARIABLES*/
	
	bool done = false; //when done = true, main game loop ends
	bool render = false; //If a new screen needs to be rendered	
	
	float gameTime = 0; //Time limit
	int frames = 0;
	int gameFPS = 0;
	
	//Game state and AI state
	int state = -1;
	int cpuState = -1;
	
	//Mode, player, opponent, and level initialization
	int mode = ONEPLAYER;
	int p1 = GHOST;
	int p2 = BMO;
	int tier = 0;
	int level = 0;
	int cpulimit = 0;

	//Only four character sprites
	const int numSprites = 4;

	Sprite player[numSprites];
	Sprite player2[numSprites];
	Sprite dropBall;
	Sprite opBall;

	//Initialize where the play ball is on screen
	int ground = (height * 2) / 3 - 8;
	int neutral = width / 2 - 8;

	/////////////////////
	/*ALLEGRO VARIABLES*/
	ALLEGRO_DISPLAY *display = NULL;
	ALLEGRO_EVENT_QUEUE *event_queue = NULL;
	ALLEGRO_FONT *font18 = NULL;
	ALLEGRO_FONT *font96 = NULL;
	ALLEGRO_FONT *font48 = NULL;
	ALLEGRO_TIMER *timer;
	ALLEGRO_BITMAP *image[numSprites];
	ALLEGRO_BITMAP *ball, *snail;
	ALLEGRO_TIMER *clock;
	ALLEGRO_SAMPLE *music[4];
	ALLEGRO_SAMPLE *sounds[4];
	ALLEGRO_SAMPLE_INSTANCE *bgm[4];
	ALLEGRO_SAMPLE_INSTANCE *sfx[4];

	/////////////////////
	/*INITIALIZATION*/
	if(!al_init())										//initialize Allegro
		return -1;

	display = al_create_display(width, height);			//create our display object

	if(!display)										//test display object
		return -1;
	////////////////
	/*ALLEGRO INIT*/
	al_install_keyboard();
	al_init_image_addon();
	al_install_audio();
	al_init_acodec_addon();
	al_init_font_addon();
	al_init_ttf_addon();
	clock = al_create_timer(1.0);

	font18 = al_load_font("sprites/Rye-Regular.ttf", 18, 0);
	font96 = al_load_font("sprites/Rye-Regular.ttf", 96, 0);
	font48 = al_load_font("sprites/Rye-Regular.ttf", 48, 0);

	//////////////////////
	//Sound Init /////////
	al_reserve_samples(10);

	music[0] = al_load_sample("sounds/title.ogg");
	music[1] = al_load_sample("sounds/select.ogg");
	music[2] = al_load_sample("sounds/playing.ogg");
	music[3] = al_load_sample("sounds/freestyle.ogg");

	sounds[0] = al_load_sample("sounds/drop.ogg");
	sounds[1] = al_load_sample("sounds/grab.ogg");
	sounds[2] = al_load_sample("sounds/win.ogg");
	sounds[3] = al_load_sample("sounds/gameover.ogg");

	//Initialize sounds to mixer channel
	for(int i = 0; i < 4; i++)
	{
		bgm[i] = al_create_sample_instance(music[i]);
		sfx[i] = al_create_sample_instance(sounds[i]);
		
		al_set_sample_instance_playmode(bgm[i], ALLEGRO_PLAYMODE_LOOP);

		al_attach_sample_instance_to_mixer(bgm[i], al_get_default_mixer());
		al_attach_sample_instance_to_mixer(sfx[i], al_get_default_mixer());

	}

	///////////////
	/*SPRITE INIT*/
	image[0] = al_load_bitmap("sprites/GHOST.png");
	image[1] = al_load_bitmap("sprites/FINN.png");
	image[2] = al_load_bitmap("sprites/JAKE.png");
	image[3] = al_load_bitmap("sprites/BMO.png");

	ball = al_load_bitmap("sprites/ball.png");
	snail = al_load_bitmap("sprites/SNAIL.png");
	for(int i = 0; i < numSprites; i++) {
		InitSprites(player[i], image[i], 64, width / 2 - 128, height / 2 - 128);
		InitSprites(player2[i], image[i], 64, width / 2 - 128, height / 2 - 128);
	}
	InitSprites(dropBall, ball, 16, neutral, ground);
	InitSprites(opBall, ball, 16, (3 * width) / 4 - 8, ground);

	event_queue = al_create_event_queue();
	timer = al_create_timer(1.0 / 60);

	al_register_event_source(event_queue, al_get_timer_event_source(timer));
	al_register_event_source(event_queue, al_get_keyboard_event_source());
	al_register_event_source(event_queue, al_get_display_event_source(display));
	/////////////
	/*GAME INIT*/
	al_start_timer(timer);
	gameTime = al_current_time();
	ChangeState(state, TITLE, clock);
	ChangeState(mode, ONEPLAYER, clock);
	ChangeCPUState(cpuState, TITLE);
	al_play_sample_instance(bgm[0]);

	//////////////////
	/*MAIN GAME LOOP*/
	while(!done)
	{
		ALLEGRO_EVENT ev;
		al_wait_for_event(event_queue, &ev);

		///////////////////
		/*KEYBOARD INPUT*/ 
		/////////////////
		if(ev.type == ALLEGRO_EVENT_KEY_DOWN)
		{
			switch(ev.keyboard.keycode)
			{
			case ALLEGRO_KEY_ESCAPE:
				done = true;
				break;
			case ALLEGRO_KEY_A:
				keys[A] = true;
				break;
			case ALLEGRO_KEY_L:
				keys[L] = true;
				break;
			case ALLEGRO_KEY_SPACE:
				keys[SPACE]=true;
				break;
			case ALLEGRO_KEY_LCTRL:
				keys[LCTRL]=true;
				break;
			case ALLEGRO_KEY_RCTRL:
				keys[RCTRL]=true;
				break;
			case ALLEGRO_KEY_S:
				keys[S]=true;
				break;
			case ALLEGRO_KEY_D:
				keys[D]=true;
				break;
			case ALLEGRO_KEY_LEFT:
				keys[LEFT]=true;
				break;
			case ALLEGRO_KEY_RIGHT:
				keys[RIGHT]=true;
				break;
			case ALLEGRO_KEY_DOWN:
				keys[DOWN]=true;
				break;
			case ALLEGRO_KEY_ENTER:
				keys[ENTER]= true;
				break;
			}
		} 
		else if(ev.type == ALLEGRO_EVENT_KEY_UP) //Checks to make sure key is let go
		{
			switch(ev.keyboard.keycode)
			{
			case ALLEGRO_KEY_A:
				keys[A] = false;
				break;
			case ALLEGRO_KEY_L:
				keys[L] = false;
				break;
			case ALLEGRO_KEY_SPACE:
				keys[SPACE]= false;
				break;
			case ALLEGRO_KEY_LCTRL:
				keys[LCTRL] = false;
				break;
			case ALLEGRO_KEY_RCTRL:
				keys[RCTRL] = false;
				break;
			case ALLEGRO_KEY_S:
				keys[S] = false;
				break;
			case ALLEGRO_KEY_D:
				keys[D]= false;
				break;
			case ALLEGRO_KEY_LEFT:
				keys[LEFT]= false;
				break;
			case ALLEGRO_KEY_RIGHT:
				keys[RIGHT]= false;
				break;
			case ALLEGRO_KEY_DOWN:
				keys[DOWN]= false;
				break;
			case ALLEGRO_KEY_ENTER:
				keys[ENTER]= false;
				break;
			}
		}
		else if(ev.type == ALLEGRO_EVENT_DISPLAY_CLOSE)
		{
			done = true; //Game loop end when window is closed out
		}

		/*CHECK GAME STATE AND ACTION*/
		else if(ev.type == ALLEGRO_EVENT_TIMER)
		{
			/*For FPS*/
			frames++;
			if(al_current_time() - gameTime >= 1)
			{
				gameTime = al_current_time();
				gameFPS = frames;
				frames = 0;
			}

			/*UPDATE THE FRAMES OF PLAYERS AND CPUS*/
			UpdateSprites(player[p1], tarFrame);
			UpdateSprites(player2[p2], cpuFrame);

			if(cpuState == PLAYING)
				UpdateCPU(player[p2], cpulimit, opBall, cpuState, state, clock, sfx);

			render = true;
			
			/////////////////////
			/*GAME STATE ACTION*/
			switch(state) 
			{
			case TITLE: //Title Screen allows to select with modes with A and L keys
				if(++player[0].frameCount >= player[0].frameDelay)
				{
					if(keys[A]) {
						if(mode == ONEPLAYER)
							mode = END;
						else
							mode--;
					}
					if(keys[L]) {
						if(mode == END)
							mode = ONEPLAYER;
						else
							mode++;
					}

					if(keys[SPACE]) {
						p1 = FINN;
						keys[SPACE] = false;
						if(mode == TWOPLAYER)
							ChangeState(state, MENU2, clock);
						else
							ChangeState(state, MENU, clock);
						al_stop_sample_instance(bgm[0]);
						al_play_sample_instance(bgm[1]);
					}
					player[0].frameCount = 0;
				}
				break;

			case MENU: //Single Player selection screen allows player 1 to select the character
				if(++player[0].frameCount >= player[0].frameDelay)
				{
					if(keys[A]) {
						if(p1 == GHOST)
							p1 = BMO;
						else
							p1--;
					}
					if(keys[L]) {
						if(p1 == BMO)
							p1 = GHOST;
						else
							p1++;
					}

					if(keys[SPACE]) {
						if(p1 == BMO)
							p2 = FINN;
						player[p1].frameDelay = 5;
						player[p2].frameDelay = 120;
						ChangeState(state, options[mode], clock);
						ChangeCPUState(cpuState, options[mode]);
						al_stop_sample_instance(bgm[1]);
						if(state == ENDLESS)
							al_play_sample_instance(bgm[3]);
						else
							al_play_sample_instance(bgm[2]);
					}
					player[0].frameCount = 0;
				}
				break;
				
			case MENU2: //Multiplayer selection screen allows player 1 and 2 to select characters
				if(++player[0].frameCount >= player[0].frameDelay)
				{
					if(keys[A]) {
						if(p1 == GHOST)
							p1 = BMO;
						else
							p1--;
					}
					if(keys[D]) {
						if(p1 == BMO)
							p1 = GHOST;
						else
							p1++;
					}

					if(keys[LEFT]) {
						if(p2 == GHOST)
							p2 = BMO;
						else
							p2--;
					}
					if(keys[RIGHT]) {
						if(p2 == BMO)
							p2 = GHOST;
						else
							p2++;
					}

					if(keys[LCTRL] && keys[RCTRL]) {
						player[p1].frameDelay = 5;
						player2[p2].frameDelay = 5;
						ChangeState(state, VERSUS, clock);
						ChangeCPUState(cpuState, VERSUS);
						al_stop_sample_instance(bgm[1]);
						al_play_sample_instance(bgm[2]);
					}
					player[0].frameCount = 0;
				}
				break;
			/////////////////////////////
			/*SINGLE PLAYER GAME STATES*/	
			case PLAYING: //What can the player do when the ball is on the ground
				NonDrop(state, clock, cpuState, sfx);
				break;
			case DROP: //What can the player do when the player is ready to drop the ball
				Drop(dropBall, state, clock, cpuState, sfx);
				break;
			
			//////////////////////////////////
			/*MULTIPLAYER PLAYER GAME STATES*/	
			//Below states make sure if the game is multiplayer
			case VERSUS:
			case DROP2:
			case WAIT2:
				if((30 - al_get_timer_count(clock)) < 0) { //Time limit check
					ChangeState(state, GAMEOVER2, clock);
					ChangeCPUState(cpuState, GAMEOVER2);
				} else 
				{
					//Player 1 Game states
					if(state == VERSUS) 
						NonDrop2(state,clock,sfx);//What can the player do when the ball is on the ground
					else if(state == DROP2)
						Drop2(dropBall,state,clock,sfx);//What can the player do when the player is ready to drop the ball
					//Player 2 Game states
					if(cpuState == VERSUS)
						NonDrop3(cpuState,clock,sfx);//What can the player do when the ball is on the ground
					else if(cpuState == WAIT2)//If Player 2 ran out of time
						break;
					else
						Drop3(opBall,cpuState,clock,sfx);//What can the player do when the player is ready to drop the ball
				}
				break;
			//////////////////////////////////
			/*ENDLESS MODE GAME STATES      */	
			//Same as single player except no time limit
			case ENDLESS:
				NonDrop(state, clock, cpuState, sfx);
				dropBall.x = neutral;
				break;
			case ENDDROP:
				Drop(dropBall, state, clock, cpuState, sfx);
				dropBall.x = neutral;
				break;
			//////////////////////////////////
			/*SINGLE PLAYER GAMEOVER SCREEN */	
			case GAMEOVER:
				al_stop_sample_instance(bgm[2]); //Stops current music
				if(score > hiscore)
					hiscore = score;
				if(keys[RCTRL] | keys[LCTRL]) {
					if(cpuState == VERSUS) {
						if(cpuscore > hiscore)
							hiscore = cpuscore;
						ChangeState(state, VERSUS, clock);
						ChangeCPUState(cpuState, VERSUS);
						al_play_sample_instance(bgm[2]);
					} else {
						ChangeState(state, PLAYING, clock);
						ChangeCPUState(cpuState, PLAYING);
						al_play_sample_instance(bgm[2]);
						if(score > cpuscore) {
							if((level % 3) == 2) {
								tier++;
							}
							level++;
							cpulimit = (level*6) + (tier * 5);
							if(p2++ == BMO) {
								p2 = GHOST;
							}

							if(p1==p2) {
								if(p2++ == BMO) {
									p2 = GHOST;
								}
							}
							if((level % 8) == 7) {
								dropBall.image = snail;
								opBall.image = snail;
							} else {
								dropBall.image = ball;
								opBall.image = ball;
							}
							player[p2].frameDelay = 30 / (tier+1);
						}
					}
					score = 0;
					cpuscore = 0;
				}
				break;
			//////////////////////////////////
			/*MULTIPLAYER GAMEOVER SCREEN   */
			case GAMEOVER2:
				al_stop_sample_instance(bgm[2]);
				if(score > hiscore)
					hiscore = score;
				if(cpuscore > hiscore)
						hiscore = cpuscore;
				if(keys[RCTRL] | keys[LCTRL]) {
					ChangeState(state, VERSUS, clock);
					ChangeCPUState(cpuState, VERSUS);
					al_play_sample_instance(bgm[2]);
					score = 0;
					cpuscore = 0;
				}
				break;
			//////////////////////////////////
			/*ENDLESS MODE GAMEOVER SCREEN  */
			case RESULT:
				if(score > hiscore)
					hiscore = score;
				if(keys[RCTRL] | keys[LCTRL]) {
					ChangeState(state, ENDLESS, clock);
				}
				score = 0;
				break;
				
			}

			///////////////////////////////////////////////////
			/*WHAT IF BOTH PLAYERS MISS IN THEIR DROP STREAK?*/
			//Game ends and tallies the score, goto appropriate GAMEOVER SCREEN
			if((state == WAIT)&&(cpuState==WAIT)) {
				al_stop_sample_instance(bgm[2]);
				ChangeState(state, GAMEOVER, clock);
				ChangeCPUState(cpuState, GAMEOVER);
				if(score > cpuscore)
					al_play_sample_instance(sfx[2]);
				else
					al_play_sample_instance(sfx[3]);

			}

			if((state == WAIT2)&&(cpuState==WAIT2)) {
				al_stop_sample_instance(bgm[2]);
				ChangeState(state, GAMEOVER2, clock);
				ChangeCPUState(cpuState, GAMEOVER2);
			}


		}

		if(render && al_is_event_queue_empty(event_queue))
		{
			render = false;

			/*DRAW SPRITES AND UPDATES LOCATION BASED ON STATES*/
			al_draw_scaled_bitmap(dropBall.image, 0, 0, dropBall.frameWidth, 
				dropBall.frameHeight, dropBall.x, dropBall.y, dropBall.frameWidth * 3 , 
				dropBall.frameHeight * 3, 0);

			switch(state) {
			case PLAYING:
			case DROP:
			case GAMEOVER:
			case WAIT:
				player[p1].x = width / 4 - 128;
				dropBall.x = width / 4 - 8;
				player[p2].x = (3 * width) / 4 - 128;
				al_draw_scaled_bitmap(opBall.image, 0, 0, opBall.frameWidth, 
				opBall.frameHeight, opBall.x, opBall.y, opBall.frameWidth * 3 , 
				opBall.frameHeight * 3, 0);
				DrawCPU(player[p2]);
			case TITLE:
			case MENU:
			case ENDLESS:
				break;
			case MENU2:
			case VERSUS:
			case DROP2:
			case GAMEOVER2:
			case WAIT2:
				player[p1].x = width / 4 - 128;
				dropBall.x = width / 4 - 8;
				player2[p2].x = (3 * width) / 4 - 128;
				al_draw_scaled_bitmap(opBall.image, 0, 0, opBall.frameWidth, 
				opBall.frameHeight, opBall.x, opBall.y, opBall.frameWidth * 3 , 
				opBall.frameHeight * 3, 0);
				DrawSprites(player2[p2]);
				break;
			}

			DrawSprites(player[p1]);
			
			/////////////////////////////////////////////
			//UI Text: Menu text, Score Display, etc.////
			/////////////////////////////////////////////
			switch(state) {
				case TITLE:
					al_draw_textf(font96, al_map_rgb(255, 255, 255), width / 8, height / 32, 0, "Drop Ball");
					al_draw_textf(font48, al_map_rgb(100, 88, 230), (width * 4) / 8, (height * 6) / 32, 0, "VERSUS");
					al_draw_textf(font18, al_map_rgb(255, 0, 0), width / 2, (height * 19)/ 24, ALLEGRO_ALIGN_CENTRE, "%s", modes[mode]);
					al_draw_textf(font18, al_map_rgb(0, 255, 0), 5, (height *15) / 16, 0, "Change Option: A, L Keys; Confirm: Space Key; Quit: Escape");
					break;
				case MENU:
					al_draw_textf(font48, al_map_rgb(100, 88, 230), width / 2, height / 32, ALLEGRO_ALIGN_CENTRE, "Select Character");
					al_draw_textf(font18, al_map_rgb(255, 255, 255), width / 2, (height * 19)/ 24, ALLEGRO_ALIGN_CENTRE, "%s", names[p1]);
					al_draw_textf(font18, al_map_rgb(0, 255, 0), 5, (height *15) / 16, 0, "Change Character: A, L Keys; Confirm: Space Key; Quit: Escape");
					break;
				case MENU2:
					al_draw_textf(font48, al_map_rgb(100, 88, 230), width / 2, height / 32, ALLEGRO_ALIGN_CENTRE, "Select Character");
					al_draw_textf(font18, al_map_rgb(255, 255, 255), width / 4, height / 5, ALLEGRO_ALIGN_CENTRE, "%s", names[p1]);
					al_draw_textf(font18, al_map_rgb(255, 255, 255), (3 * width) / 4, height / 5, ALLEGRO_ALIGN_CENTRE, "%s", names[p2]);
					al_draw_textf(font18, al_map_rgb(0, 255, 0), 5, (height *14) / 16, 0, "Change Player 1: A, D Keys; Confirm: Hold LCTRL; Quit: Escape");
					al_draw_textf(font18, al_map_rgb(0, 255, 0), 5, (height *15) / 16, 0, "Change Player 2: Left, Right Arrows; Confirm: Hold RCTRL");
					break;
				case GAMEOVER:
					if(score > cpuscore) {
						al_draw_textf(font96, al_map_rgb(255, 255, 255), width / 2, height / 32, ALLEGRO_ALIGN_CENTRE, "YOU WIN!");
					} else if(score < cpuscore) {
							al_draw_textf(font96, al_map_rgb(255, 255, 255), width / 2, height / 32, ALLEGRO_ALIGN_CENTRE, "YOU LOSE!");
					}
					else {
							al_draw_textf(font96, al_map_rgb(255, 255, 255), width / 2, height / 32, ALLEGRO_ALIGN_CENTRE, "TIE!");
					}
					al_draw_textf(font18, al_map_rgb(255, 1, 255), width / 2, (height * 7)/ 9, ALLEGRO_ALIGN_CENTRE, "Final Score: %i", score);
					al_draw_textf(font18, al_map_rgb(255, 0, 0), width / 2, (height * 7)/ 8, ALLEGRO_ALIGN_CENTRE, "Press CTRL to Try Again");
					break;
				case GAMEOVER2:
					if(score > cpuscore) {
							al_draw_textf(font96, al_map_rgb(255, 255, 255), width / 2, height / 32, ALLEGRO_ALIGN_CENTRE, "P1 WINS!");
					} else if(score < cpuscore) {
							al_draw_textf(font96, al_map_rgb(255, 255, 255), width / 2, height / 32, ALLEGRO_ALIGN_CENTRE, "P2 WINS!");
					}
					else {
							al_draw_textf(font96, al_map_rgb(255, 255, 255), width / 2, height / 32, ALLEGRO_ALIGN_CENTRE, "TIE!");
					}
					al_draw_textf(font18, al_map_rgb(255, 0, 0), width / 2, (height * 7)/ 8, ALLEGRO_ALIGN_CENTRE, "Press CTRL to Try Again");
					break;
				case RESULT:
					al_draw_textf(font96, al_map_rgb(255, 255, 255), width / 2, height / 32, ALLEGRO_ALIGN_CENTRE, "GAME OVER");
					al_draw_textf(font18, al_map_rgb(255, 1, 255), width / 2, (height * 7)/ 9, ALLEGRO_ALIGN_CENTRE, "Final Score: %i", score);
					al_draw_textf(font18, al_map_rgb(255, 0, 0), width / 2, (height * 7)/ 8, ALLEGRO_ALIGN_CENTRE, "Press CTRL to Try Again");
					break;
				case ENDLESS:
				case ENDDROP:
					al_draw_textf(font48, al_map_rgb(255, 0, 255), width / 2, 5, ALLEGRO_ALIGN_CENTRE, "My High Score: %i", hiscore);
					al_draw_textf(font18, al_map_rgb(255, 255, 255), width / 2, (height * 20)/ 24, ALLEGRO_ALIGN_CENTRE, "Score: %i", score);
					break;
				case WAIT2:
					al_draw_textf(font18, al_map_rgb(255, 0, 0), width / 4, (height * 18)/ 24, ALLEGRO_ALIGN_CENTRE, "MISS");
				case DROP2:
				case VERSUS:
					if(cpuState == WAIT2)
						al_draw_textf(font18, al_map_rgb(255, 0, 0), (3* width) / 4, (height * 18)/ 24, ALLEGRO_ALIGN_CENTRE, "MISS");
					//UNCOMMENT TO DEBUG FPS: al_draw_textf(font18, al_map_rgb(255, 0, 255), 5, 72, 0, "FPS: %i", gameFPS);
					al_draw_textf(font48, al_map_rgb(255, 255, 0), width / 2, 5, ALLEGRO_ALIGN_CENTRE, "Time: %i", 30 - al_get_timer_count(clock));
					al_draw_textf(font18, al_map_rgb(255, 0, 255), width / 2, (22* height) / 24, ALLEGRO_ALIGN_CENTRE, "My High Score: %i", hiscore);
					al_draw_textf(font18, al_map_rgb(255, 255, 255), width / 4, (height * 20)/ 24, ALLEGRO_ALIGN_CENTRE, " 1P Score: %i", score);
					al_draw_textf(font18, al_map_rgb(255, 255, 255), (3* width) / 4, (height * 20)/ 24, ALLEGRO_ALIGN_CENTRE, "2P Score: %i", cpuscore);
					break;
				case WAIT:
					al_draw_textf(font18, al_map_rgb(255, 0, 0), width / 4, (height * 18)/ 24, ALLEGRO_ALIGN_CENTRE, "MISS");
				default:
					if(cpuState == WAIT)
						al_draw_textf(font18, al_map_rgb(255, 0, 0), (3* width) / 4, (height * 18)/ 24, ALLEGRO_ALIGN_CENTRE, "MISS");
					//UNCOMMENT TO DEBUG FPS: al_draw_textf(font18, al_map_rgb(255, 0, 255), 5, 72, 0, "FPS: %i", gameFPS);
					al_draw_textf(font48, al_map_rgb(255, 255, 0), width / 2, 5, ALLEGRO_ALIGN_CENTRE, "Time: %i", 30 - al_get_timer_count(clock));
					al_draw_textf(font18, al_map_rgb(255, 0, 255), width / 2, (22* height) / 24, ALLEGRO_ALIGN_CENTRE, "My High Score: %i", hiscore);
					al_draw_textf(font18, al_map_rgb(255, 255, 255), width / 4, (height * 20)/ 24, ALLEGRO_ALIGN_CENTRE, "Score: %i", score);
					al_draw_textf(font18, al_map_rgb(255, 255, 255), (3* width) / 4, (height * 20)/ 24, ALLEGRO_ALIGN_CENTRE, "Opponent's Score: %i", cpuscore);
					break;
			}
			al_flip_display();
			al_clear_to_color(al_map_rgb(0,0,0));
		}
	} //MAIN GAME LOOP ENDS

	///////////////////////////
	/*DESTROY ALLEGRO OBJECTS*/
	///////////////////////////
	al_destroy_timer(clock);
	for(int i = 0; i < numSprites; i++)
		al_destroy_bitmap(image[i]);
	al_destroy_bitmap(ball);
	al_destroy_bitmap(snail);
	for(int i = 0; i < 4; i++) {
		al_destroy_sample_instance(bgm[i]);
		al_destroy_sample(music[i]);
		
		al_destroy_sample_instance(sfx[i]);
		al_destroy_sample(sounds[i]);
	}
	al_destroy_font(font18);
	al_destroy_font(font96);
	al_destroy_font(font48);
	al_destroy_event_queue(event_queue);
	al_destroy_timer(timer);
	al_destroy_display(display);

	return 0;
}

/*Initialize Sprites to default values*/
void InitSprites(Sprite &sprite, ALLEGRO_BITMAP *image, int size, int x, int y)
{
	sprite.x = x;
	sprite.y = y;

	sprite.curFrame = 0;
	sprite.frameCount = 0;
	sprite.frameDelay = 5;
	sprite.frameWidth = size;
	sprite.frameHeight = size;
	sprite.animationColumns = 4;
	sprite.animationDirection = 1;

	sprite.image = image;
}

/*Changes sprite frame once frame count == frame delay*/
void UpdateSprites(Sprite &sprite, int newFrame)
{
	if(++sprite.frameCount >= sprite.frameDelay)
	{
		sprite.curFrame = newFrame;

		sprite.frameCount = 0;
	}

}

/*DROP BALL CPU AI and update appropriate sprite frame*/
void UpdateCPU(Sprite &sprite, int cpulimit, Sprite &opBall, int &cpuState, int &state, ALLEGRO_TIMER *clock, ALLEGRO_SAMPLE_INSTANCE **sfx)
{
 	bool drop = false;
	int ground = (height * 2) / 3 - 8;
	if((30 - al_get_timer_count(clock)) < 0) {
		ChangeState(state, GAMEOVER, clock);
		ChangeCPUState(cpuState, GAMEOVER);
		if(score > cpuscore)
			al_play_sample_instance(sfx[2]);
		else
			al_play_sample_instance(sfx[3]);
	}
	if(++sprite.frameCount >= sprite.frameDelay)
	{
		switch(sprite.curFrame) 
		{
		case 0:
			sprite.curFrame++;
			opBall.y = ground;
			break;
		case 1:
			sprite.curFrame++;
			opBall.y = ground;
			break;
		case 2:
			sprite.curFrame++;
			if(cpuscore < cpulimit) {
				opBall.y = ground-32;
			} else {
				if(cpulimit != 0)
					ChangeCPUState(cpuState, WAIT);
			}
			break;
		case 3:
			if(cpulimit == 0)
				sprite.curFrame = 0;
			else {
				sprite.curFrame = 7;
				al_play_sample_instance(sfx[1]);
			}
			opBall.y = ground;
			break;
		case 7:
			sprite.curFrame = 0;
			cpuscore++;
			opBall.y = ground;
			al_play_sample_instance(sfx[0]);
			break;
		}

		sprite.frameCount = 0;
	}

}

/*Draw player 1 sprite on screen*/
void DrawSprites(Sprite &sprite)
{
	int fx = (sprite.curFrame % sprite.animationColumns) * sprite.frameWidth;
	int fy = (sprite.curFrame / sprite.animationColumns) * sprite.frameHeight;

	al_draw_scaled_bitmap(sprite.image, fx, fy, sprite.frameWidth, 
		sprite.frameHeight,sprite.x, sprite.y, sprite.frameWidth * 4 , 
		sprite.frameHeight * 4 , 0);

}

/*Draw player 2 or CPU sprite on screen*/
void DrawCPU(Sprite &sprite)
{
	int fx = (sprite.curFrame % sprite.animationColumns) * sprite.frameWidth;
	int fy = (sprite.curFrame / sprite.animationColumns) * sprite.frameHeight;

	al_draw_scaled_bitmap(sprite.image, fx, fy, sprite.frameWidth, 
		sprite.frameHeight,sprite.x, sprite.y, sprite.frameWidth * 4 , 
		sprite.frameHeight * 4 , 0);

}

/* NONDROP functions does Player animations when the ball isn't grabbed (Not ready to be Dropped) /
/* DROP functions does Player animations when the ball is grabbed (READY TO BE DROPPED)          */
void NonDrop(int &state, ALLEGRO_TIMER *clock, int &cpustate, ALLEGRO_SAMPLE_INSTANCE **sfx){
	if((30 - al_get_timer_count(clock)) < 0) {
		ChangeState(state, GAMEOVER, clock);
		ChangeCPUState(cpustate, GAMEOVER);
		if(score > cpuscore)
			al_play_sample_instance(sfx[2]);
		else
			al_play_sample_instance(sfx[3]);
	}
	if(!(keys[A]  & keys[L] & keys[SPACE])){
		tarFrame = 0; //Standing animation
	}

	if(keys[A]) {
		tarFrame = 4; //Left Crouch
	}
	if(keys[L]) {
		tarFrame = 5; //Right Crouch
	}
	if(keys[A]  & keys[L]){ //Crouch
		tarFrame = 1;
	}

	if(keys[A]  & keys[L] & keys[SPACE]){ //Ball grabbed
		tarFrame = 2;
		if(state == ENDLESS)
			ChangeState(state, ENDDROP, clock);
		else
			ChangeState(state, DROP, clock);
		al_play_sample_instance(sfx[1]);
	}
}

void Drop(Sprite &ball, int &state, ALLEGRO_TIMER *clock, int &cpustate, ALLEGRO_SAMPLE_INSTANCE **sfx){
	int ground = (height * 2) / 3 - 8;
	int neutral = width / 4 - 8;
	if((30 - al_get_timer_count(clock)) < 0) {
		ChangeState(state, GAMEOVER, clock);
		ChangeCPUState(cpustate, GAMEOVER);
		if(score > cpuscore)
			al_play_sample_instance(sfx[2]);
		else
			al_play_sample_instance(sfx[3]);
	}
	if(!(keys[A]  & keys[L] & keys[SPACE])){
		tarFrame = 7;
		ball.y = ground-32; //Location of the ball on screen
		ball.x = neutral-20; //Location of the ball on screen
	}

	if(keys[A]) {
		tarFrame = 4;
		ball.y = ground-8;
		ball.x = neutral;
	}
	if(keys[L]) {
		tarFrame = 5;
		ball.y = ground-8;
		ball.x = neutral-20;
	}

	if(keys[A] & keys[L]){
		tarFrame = 1;
		ball.x = neutral;
		ball.y = ground;
	}
	if(keys[A]  & keys[L] & keys[SPACE]){
		tarFrame = 6;
		ball.x = neutral;
		ball.y = ground;
	}
	if(!keys[SPACE]){
		ball.x = neutral;
		ball.y = ground;
		if(tarFrame == 7) { //ONLY A SUCCESSFUL DROP IF DROPPED STANDING UP STRAIGHT
			score++;
			if(state == ENDDROP)
				ChangeState(state, ENDLESS, clock);
			else
				ChangeState(state, PLAYING, clock);
		}
		else {
			if(state == ENDDROP)
				ChangeState(state, RESULT, clock);
			else
				ChangeState(state, WAIT, clock);
		}
		al_play_sample_instance(sfx[0]);
	}
}

/* NONDROP2 AND DROP2 FUNCTIONS ARE FOR PLAYER 1 DURING MULTIPLAYER*/
void NonDrop2(int &state, ALLEGRO_TIMER *clock, ALLEGRO_SAMPLE_INSTANCE **sfx){
	if(!(keys[A]  & keys[D] & keys[S])){
		tarFrame = 0;
	}

	if(keys[A]) {
		tarFrame = 4;
	}
	if(keys[D]) {
		tarFrame = 5;
	}
	if(keys[A]  & keys[D]){
		tarFrame = 1;
	}

	if(keys[A]  & keys[D] & keys[S]){
		tarFrame = 2;
		ChangeState(state, DROP2, clock);
		al_play_sample_instance(sfx[1]);
	}
}

void Drop2(Sprite &ball, int &state, ALLEGRO_TIMER *clock, ALLEGRO_SAMPLE_INSTANCE **sfx){
	int ground = (height * 2) / 3 - 8;
	int neutral = width / 4 - 8;
	if(!(keys[A]  & keys[D] & keys[S])){
		tarFrame = 7;
		ball.y = ground-32;
		ball.x = neutral-20;
	}

	if(keys[A]) {
		tarFrame = 4;
		ball.y = ground-8;
		ball.x = neutral;
	}
	if(keys[D]) {
		tarFrame = 5;
		ball.y = ground-8;
		ball.x = neutral-20;
	}

	if(keys[A] & keys[D]){
		tarFrame = 1;
		ball.x = neutral;
		ball.y = ground;
	}
	if(keys[A]  & keys[D] & keys[S]){
		tarFrame = 6;
		ball.x = neutral;
		ball.y = ground;
	}
	if(!keys[S]){
		ball.x = neutral;
		ball.y = ground;
		if(tarFrame == 7) {
			score++;
			ChangeState(state, VERSUS, clock);
		}
		else {
			ChangeState(state, WAIT2, clock);
		}
		al_play_sample_instance(sfx[0]);
	}
}

/* NONDROP3 AND DROP3 FUNCTIONS ARE FOR PLAYER 2 DURING MULTIPLAYER*/
void NonDrop3(int &state, ALLEGRO_TIMER *clock, ALLEGRO_SAMPLE_INSTANCE **sfx){
	if(!(keys[LEFT]  & keys[RIGHT] & keys[DOWN])){
		cpuFrame = 0;
	}

	if(keys[LEFT]) {
		cpuFrame = 4;
	}
	if(keys[RIGHT]) {
		cpuFrame = 5;
	}
	if(keys[LEFT]  & keys[RIGHT]){
		cpuFrame = 1;
	}

	if(keys[LEFT]  & keys[RIGHT] & keys[DOWN]){
		cpuFrame = 2;
		ChangeCPUState(state, DROP2);
		al_play_sample_instance(sfx[1]);
	}
}

void Drop3(Sprite &ball, int &state, ALLEGRO_TIMER *clock, ALLEGRO_SAMPLE_INSTANCE **sfx){
	int ground = (height * 2) / 3 - 8;
	int neutral = (3 * width) / 4 - 8;
	if(!(keys[LEFT]  & keys[RIGHT] & keys[DOWN])){
		cpuFrame = 7;
		ball.y = ground-32;
		ball.x = neutral-20;
	}

	if(keys[LEFT]) {
		cpuFrame = 4;
		ball.y = ground-8;
		ball.x = neutral;
	}
	if(keys[RIGHT]) {
		cpuFrame = 5;
		ball.y = ground-8;
		ball.x = neutral-20;
	}

	if(keys[LEFT] & keys[RIGHT]){
		cpuFrame = 1;
		ball.x = neutral;
		ball.y = ground;
	}
	if(keys[LEFT]  & keys[RIGHT] & keys[DOWN]){
		cpuFrame = 6;
		ball.x = neutral;
		ball.y = ground;
	}
	if(!keys[DOWN]){
		ball.x = neutral;
		ball.y = ground;
		if(cpuFrame == 7) {
			cpuscore++;
			ChangeCPUState(state, VERSUS);
		}
		else {
			ChangeCPUState(state, WAIT2);
		}
		al_play_sample_instance(sfx[0]);
	}
}

/*CHANGE GAME STATE, DOES PRE/POST ACTIONS, SENDS DEBUG INFORMATION*/
void ChangeState(int &state, int newState, ALLEGRO_TIMER *clock)
{
	switch(state) {
	case TITLE:
		std::cout << "Leaving the TITLE state\n";
		break;
	case PLAYING:
		std::cout << "Leaving the PLAYING state\n";
		break;
	case ENDLESS:
		std::cout << "Leaving the ENDLESS state\n";
		break;
	case ENDDROP:
		std::cout << "Leaving the ENDDROP state\n";
		break;
	case VERSUS:
		std::cout << "Leaving the VERSUS state\n";
		al_start_timer(clock);
		break;
	case DROP:
		std::cout << "Leaving the DROP state\n";
		break;
	case DROP2:
		std::cout << "Leaving the DROP2 state\n";
		break;
	case GAMEOVER:
		std::cout << "Leaving the GAMEOVER state\n";
		al_set_timer_count(clock, 0);
		al_start_timer(clock);
		break;
	case WAIT:
		std::cout << "Leaving the WAIT state\n";
		break;
	case GAMEOVER2:
		std::cout << "Leaving the GAMEOVER2 state\n";
		al_set_timer_count(clock, 0);
		al_start_timer(clock);
		break;
	case MENU:
		std::cout << "Leaving the MENU state\n";
		break;
	case MENU2:
		std::cout << "Leaving the MENU2 state\n";
		break;
	case WAIT2:
		std::cout << "Leaving the WAIT state\n";
		break;
	}

	state = newState;

	switch(state) {
	case TITLE:
		std::cout << "Entering the TITLE state\n";
		break;
	case PLAYING:
		std::cout << "Entering the PLAYING state\n";
		al_start_timer(clock);
		break;
	case ENDLESS:
		std::cout << "Entering the ENDLESS state\n";
		al_stop_timer(clock);
		break;
	case ENDDROP:
		std::cout << "Entering the ENDDROP state\n";
		break;
	case VERSUS:
		std::cout << "Entering the VERSUS state\n";
		al_start_timer(clock);
		break;
	case DROP:
		std::cout << "Entering the DROP state\n";
		break;
	case DROP2:
		std::cout << "Entering the DROP2 state\n";
		break;
	case GAMEOVER:
		std::cout << "Entering the GAMEOVER state\n";
		al_stop_timer(clock);
		break;
	case WAIT:
		std::cout << "Entering the WAIT state\n";
		break;
	case WAIT2:
		std::cout << "Entering the WAIT2 state\n";
		break;
	case GAMEOVER2:
		std::cout << "Entering the GAMEOVER2 state\n";
		al_stop_timer(clock);
		break;
	case MENU:
		std::cout << "Entering the MENU state\n";
		break;
	case MENU2:
		std::cout << "Entering the MENU state\n";
		break;
	}
}

void ChangeCPUState(int &state, int newState)
{
	switch(state) {
	case TITLE:
		std::cout << "Opponent leaving the TITLE state\n";
		break;
	case PLAYING:
		std::cout << "Opponent leaving the PLAYING state\n";
		break;
	case DROP:
		std::cout << "Opponent leaving the DROP state\n";
		break;
	case GAMEOVER:
		std::cout << "Opponent leaving the GAMEOVER state\n";
		break;
	case GAMEOVER2:
		std::cout << "Opponent leaving the GAMEOVER2 state\n";
		break;
	case VERSUS:
		std::cout << "Opponent leaving the VERSUS state\n";
		break;
	case DROP2:
		std::cout << "Opponent leaving the DROP2 state\n";
		break;
	case WAIT2:
		std::cout << "Opponent leaving the WAIT2 state\n";
		break;
	}

	state = newState;

	switch(state) {
	case TITLE:
		std::cout << "Opponent entering the TITLE state\n";
		break;
	case PLAYING:
		std::cout << "Opponent entering the PLAYING state\n";
		break;
	case DROP:
		std::cout << "Opponent entering the DROP state\n";
		break;
	case GAMEOVER:
		std::cout << "Opponent entering the GAMEOVER state\n";
		break;
	case GAMEOVER2:
		std::cout << "Opponent entering the GAMEOVER2 state\n";
		break;
	case VERSUS:
		std::cout << "Opponent entering the VERSUS state\n";
		break;
	case DROP2:
		std::cout << "Opponent entering the DROP2 state\n";
		break;
	case WAIT2:
		std::cout << "Opponent Entering the WAIT2 state\n";
		break;
	}
}