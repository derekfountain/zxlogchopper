/*
 * SDCC compiler with the (graphics capable) classic lib:
 *
 *  zcc +zx -compiler=sdcc zxlogchopper.c -o zxlogchopper -lndos -create-app --c-code-in-asm -m --list --opt-code-speed
 */

#include <stdio.h>
#include <graphics.h>
#include <intrinsic.h>
#include <im2.h>
#include <conio.h>
#include <string.h>
#include <arch/zx/spectrum.h>
#include <input.h>
#include <arch/zx/zx_input.h>
#include <sound.h>

M_BEGIN_ISR(myisr)
{
  /* Nothing here, but I need a HALT and don't need the Spectrum ROM routine */
}
M_END_ISR

typedef enum
{
  CHOPPER_VERY_FAST = 5,    // The numbers are used in a loop addition, found empirically
  CHOPPER_FAST      = 4,
  CHOPPER_MEDIUM    = 3,
  CHOPPER_SLOW      = 2,
}
CHOPPER_SPEED;

#define MAX_NUM_CUT_POINTS 4

typedef struct round_data
{
  uint8_t       active;        // For testing

  uint8_t       log_x_pos;     // Not including the left edge offset
  uint8_t       log_length;    // Max of 180

  CHOPPER_SPEED speed;

  uint8_t       cut_points[MAX_NUM_CUT_POINTS];
}
ROUND_DATA;

#define NUM_ROUNDS        50

#define EXTREME_LEFT 0
#define MEDIUM_LEFT  20
#define LEFT         40

#define LONG_LOG          180
#define MEDIUM_LONG_LOG   150
#define MEDIUM_LOG        110
#define MEDIUM_SHORT_LOG   80
#define SHORT_LOG          50

ROUND_DATA round[] = 
{
  {1, EXTREME_LEFT,  LONG_LOG,         CHOPPER_SLOW,     {  90,   0,   0,   0 } },
  {1, EXTREME_LEFT,  LONG_LOG,         CHOPPER_SLOW,     {  90, 120,   0,   0 } },
  {1, EXTREME_LEFT,  LONG_LOG,         CHOPPER_SLOW,     {  90, 120, 150,   0 } },
  {1, EXTREME_LEFT,  LONG_LOG,         CHOPPER_SLOW,     {  45,  90, 120, 150 } },
  {1, EXTREME_LEFT,  LONG_LOG,         CHOPPER_MEDIUM,   { 145,   0,   0,   0 } },
  {1, EXTREME_LEFT,  LONG_LOG,         CHOPPER_MEDIUM,   { 120, 145,   0,   0 } },

  {1, MEDIUM_LEFT,   MEDIUM_LONG_LOG,  CHOPPER_SLOW,     {  80, 135,   0,   0 } },
  {1, MEDIUM_LEFT,   MEDIUM_LOG,       CHOPPER_SLOW,     {  25,  65,  80,  95 } },
  {1, LEFT,          MEDIUM_LOG,       CHOPPER_MEDIUM,   {  25,  65,  80,  95 } },
  {1, LEFT,          SHORT_LOG,        CHOPPER_FAST,     {  40,   0,   0,   0 } },
  {1, EXTREME_LEFT,  LONG_LOG,         CHOPPER_SLOW,     { 100, 120, 140, 165 } },
  {1, LEFT+30,       SHORT_LOG,        CHOPPER_MEDIUM,   {  30,  40,   0,   0 } },
  
  {1, EXTREME_LEFT,  LONG_LOG,         CHOPPER_SLOW,     {  25,  60, 100, 125 } },
  {1, LEFT,          125,              CHOPPER_SLOW,     { 100,   0,   0,   0 } },
  {1, MEDIUM_LEFT,   LONG_LOG,         CHOPPER_SLOW,     {  80, 140,   0,   0 } },
  {1, EXTREME_LEFT,  MEDIUM_LONG_LOG,  CHOPPER_MEDIUM,   {  80, 140,   0,   0 } },
  {1, EXTREME_LEFT,  MEDIUM_SHORT_LOG, CHOPPER_VERY_FAST,{  60,   0,   0,   0 } },
  {1, EXTREME_LEFT,  LONG_LOG,         CHOPPER_SLOW,     {  22,  67, 112, 158 } },

  {1, MEDIUM_LEFT,   MEDIUM_LONG_LOG,  CHOPPER_MEDIUM,   {  25,   0,   0,   0 } },
  {1, MEDIUM_LEFT,   LONG_LOG,         CHOPPER_FAST,     {  40, 100, 140, 160 } },
  {1, EXTREME_LEFT,  MEDIUM_LOG,       CHOPPER_SLOW,     {  70,  85, 100,   0 } },
  {1, LEFT,          MEDIUM_LOG,       CHOPPER_VERY_FAST,{  30,  90,   0,   0 } },
  {1, LEFT+30,       SHORT_LOG,        CHOPPER_VERY_FAST,{  25,   0,   0,   0 } },
  {1, EXTREME_LEFT,  LONG_LOG,         CHOPPER_VERY_FAST,{  20, 105, 160,   0 } },

  {1, MEDIUM_LEFT,   MEDIUM_SHORT_LOG, CHOPPER_SLOW,     {  10,  70,   0,   0 } },
  {1, MEDIUM_LEFT,   MEDIUM_SHORT_LOG, CHOPPER_MEDIUM,   {  10,  40,  70,   0 } },
  {1, MEDIUM_LEFT,   SHORT_LOG,        CHOPPER_FAST,     {  40,   0,   0,   0 } },
  {1, MEDIUM_LEFT,   SHORT_LOG,        CHOPPER_FAST,     {  30,   0,   0,   0 } },
  {1, EXTREME_LEFT,  SHORT_LOG,        CHOPPER_MEDIUM,   {  20,   0,   0,   0 } },
  {1, EXTREME_LEFT,  LONG_LOG,         CHOPPER_VERY_FAST,{  20,  60,  90, 150 } },

  {1, MEDIUM_LEFT,   MEDIUM_LONG_LOG,  CHOPPER_MEDIUM,   { 110, 130,   0,   0 } },
  {1, LEFT,          MEDIUM_SHORT_LOG, CHOPPER_FAST,     {  40,  50,   0,   0 } },
  {1, LEFT+80,       MEDIUM_SHORT_LOG, CHOPPER_VERY_FAST,{  15,  65,   0,   0 } },
  {1, LEFT+20,       MEDIUM_SHORT_LOG, CHOPPER_MEDIUM,   {  10,  50,  70,   0 } },
  {1, LEFT,          MEDIUM_LOG,       CHOPPER_MEDIUM,   {  20,  60,  70,  80 } },
  {1, MEDIUM_LEFT,   LONG_LOG,         CHOPPER_SLOW,     {  10,  25,  40,  55 } },

  {1, MEDIUM_LEFT,   LONG_LOG,         CHOPPER_SLOW,     { 125, 140, 155, 170 } },
  {1, LEFT,          MEDIUM_SHORT_LOG, CHOPPER_VERY_FAST,{  40,  60,   0,   0 } },
  {1, LEFT,          MEDIUM_LONG_LOG,  CHOPPER_MEDIUM,   {  27,  55,  90, 100 } },
  {1, EXTREME_LEFT,  LONG_LOG,         CHOPPER_MEDIUM,   {  27,  55,  90, 100 } },
  {1, MEDIUM_LEFT,   SHORT_LOG,        CHOPPER_SLOW,     {  25,  35,  40,   0 } },
  {1, LEFT,          SHORT_LOG,        CHOPPER_MEDIUM,   {  10,  25,  00,   0 } },

  {1, MEDIUM_LEFT,   LONG_LOG,         CHOPPER_MEDIUM,   { 105, 118, 150, 160 } },
  {1, EXTREME_LEFT,  SHORT_LOG,        CHOPPER_FAST,     {  43,   0,   0,   0 } },
  {1, EXTREME_LEFT,  MEDIUM_LOG,       CHOPPER_SLOW,     {  15,  75,  95,   0 } },
  {1, LEFT,          MEDIUM_LONG_LOG,  CHOPPER_FAST,     {  80,  95, 125,   0 } },
  {1, MEDIUM_LEFT+60,MEDIUM_LOG,       CHOPPER_MEDIUM,   {  12,  30,  45,  60 } },
  {1, EXTREME_LEFT,  MEDIUM_LOG,       CHOPPER_SLOW,     {  25,  55,  90, 100 } },

  {1, LEFT,          SHORT_LOG,        CHOPPER_FAST,     {  25,   0,   0,   0 } },
  {1, MEDIUM_LEFT,   LONG_LOG,         CHOPPER_FAST,     {  22,  67, 112, 158 } },
};


/*
 * Left edge is the closest the left side of the log can be to the left border.
 * There needs to be a bit of space so the player has time to see the log.
 */
#define LEFT_EDGE ((uint8_t)50)

#define TOP_EDGE   ((uint8_t)64)
#define HEIGHT     ((uint8_t)16)

void draw_round( uint8_t round_num )
{
  drawb( LEFT_EDGE + round[round_num].log_x_pos,
	 TOP_EDGE,
	 round[round_num].log_length,
	 HEIGHT);

  /* Draw little triangles for the cutting points */
  for( uint8_t point_num = 0; point_num < MAX_NUM_CUT_POINTS; point_num++ )
  {
    if( round[round_num].cut_points[point_num] == 0 )
      break;

    const uint8_t cut_point_y_pos  = 2;
    const uint8_t cut_point_height = 5;

    const uint8_t log_left_side_px = LEFT_EDGE + round[round_num].log_x_pos;
    const uint8_t cut_point_px     = log_left_side_px + round[round_num].cut_points[point_num];

    draw( cut_point_px,     TOP_EDGE + HEIGHT + cut_point_y_pos,
	  cut_point_px + 2, TOP_EDGE + HEIGHT + cut_point_y_pos + cut_point_height );
    draw( cut_point_px + 2, TOP_EDGE + HEIGHT + cut_point_y_pos + cut_point_height,
	  cut_point_px - 2, TOP_EDGE + HEIGHT + cut_point_y_pos + cut_point_height );
    draw( cut_point_px - 2, TOP_EDGE + HEIGHT + cut_point_y_pos + cut_point_height,
	  cut_point_px,     TOP_EDGE + HEIGHT + cut_point_y_pos );
  }
}

uint8_t play_round( uint8_t round_num, uint16_t *score )
{
  uint8_t game_over = 0;

  /* Wait while player is holding space */
  while( in_KeyPressed( IN_KEY_SCANCODE_SPACE ) != 0 );

  draw_round( round_num );
  bit_beepfx(BEEPFX_SELECT_1);

  /* Note number of cut points this round, that's how many cuts the player gets */
  uint8_t num_cuts_allowed = 0;
  for( uint8_t point_num = 0; point_num < MAX_NUM_CUT_POINTS; point_num++ )
  {
    if( round[round_num].cut_points[point_num] == 0 )
      break;

    num_cuts_allowed++;
  }

  uint8_t player_cut_points[MAX_NUM_CUT_POINTS] = {0};

  uint8_t space_down          = 0;
  uint8_t previous_space_down = 0;
  uint8_t num_cuts_played     = 0;

  /* Zip the chopper across the screen */
  for( uint8_t x=0; x<(255-CHOPPER_VERY_FAST); x+=round[round_num].speed )
  {
    const uint8_t CHOPPER_Y_POS  = 32;
    const uint8_t CHOPPER_HEIGHT = CHOPPER_Y_POS+8;

    draw(x,  CHOPPER_Y_POS, x,  CHOPPER_HEIGHT);
    draw(x+1,CHOPPER_Y_POS, x+1,CHOPPER_HEIGHT);

    space_down = (in_KeyPressed( IN_KEY_SCANCODE_SPACE ) != 0);

    if( (space_down == 0) && (previous_space_down == 0) )
    {
      /* Space untouched */
    }
    else if( (space_down == 1) && (previous_space_down == 0) )
    {
      /* Space has gone down */
      previous_space_down = 1;
      
      bit_beepfx(BEEPFX_PICK);

      /* Chopper only works when over the log (not off the ends), ignore otherwise */
      if( num_cuts_played < num_cuts_allowed )
      {
	if( (x > (LEFT_EDGE + round[round_num].log_x_pos))
	    && 
	     (x < (LEFT_EDGE + round[round_num].log_x_pos + round[round_num].log_length)) )
	{
	  zx_border( INK_RED );

	  uint8_t cut_point = x - (LEFT_EDGE + round[round_num].log_x_pos);

	  /* Draw the cut point */
	  xordraw( LEFT_EDGE + round[round_num].log_x_pos + cut_point, TOP_EDGE,
		   LEFT_EDGE + round[round_num].log_x_pos + cut_point, TOP_EDGE+HEIGHT-1 );

	  player_cut_points[num_cuts_played] = cut_point;

	  num_cuts_played++;
	}
	else
	{
	  /* Missed the log completely, ignore */
	}
      }
    }
    else if( (space_down == 1) && (previous_space_down == 1) )
    {
      /* Space was down and is still dowm, it's held */
    }
    else if( (space_down == 0) && (previous_space_down == 1) )
    {
      /* Space has been released */
      previous_space_down = 0;
      zx_border( INK_GREEN );
    }

#define DEBUG 0
#if DEBUG
    gotoxy(0,0);
    printf("A=%d, P=%d, X=%d SD=%d PSD=%d", num_cuts_allowed, num_cuts_played,
	   x, space_down, previous_space_down);
#endif

    intrinsic_halt();
    
    undraw(x,  CHOPPER_Y_POS, x,  CHOPPER_HEIGHT);
    undraw(x+1,CHOPPER_Y_POS, x+1,CHOPPER_HEIGHT);    
  }

  /*
   * Force border back green in case chopper went down right at the extreme
   * right of the log, in which case it's stuck on red
   */
  zx_border( INK_GREEN );
    
  uint16_t round_score = 0;
  uint8_t report_line = 13;
  if( num_cuts_played != num_cuts_allowed )
  {
    /* Player didn't cut the log into the correct number of pieces */
    printf("%c%c%c Log wasn't even cut correctly! Hopeless!",
	   22,32+report_line,32+12);
    game_over = 1;
    report_line++;
  }
  else
  {
    /*
     * Scoring. Worked out empirically until it seemed fair. :)
     */
    for( uint8_t point_num=0; point_num<num_cuts_allowed; point_num++ )
    {
      /* Work out the players effort */
      const uint16_t correct_point        = round[round_num].cut_points[point_num];
      const uint16_t player_point         = player_cut_points[point_num];
      const uint16_t player_inaccuracy    = abs(correct_point-player_point);
    
      round_score += (100 - player_inaccuracy);
      if( player_inaccuracy == 0 )
	round_score *= 2;

      const char* rating;
      if( player_inaccuracy == 0 )
	rating = "Spot on, bonus awarded!";
      else if( player_inaccuracy <= 3 )
	rating = "Excellent!";
      else if( player_inaccuracy <= 10 )
	rating = "Fine job";
      else if( player_inaccuracy <= 20 )
	rating = "It'll do";
      else if( player_inaccuracy <= 35 )
	rating = "Do better, or else!";
      else
      {
	rating = "Unacceptable!";
	game_over = 1;
      }

#if DEBUG      
      printf("%c%c%c%d) Cut point: %3dm Cut made: %3dm Inaccuracy: %3d %s",
	     22,32+report_line,32+0,
	     point_num+1,
	     correct_point,
	     player_point,
	     player_inaccuracy,
	     rating
	);
#else      
      if ( player_inaccuracy == 0 )
      {
	printf("%c%c%c%d) %s",
	       22,32+report_line,32+14,
	       point_num+1,
	       rating );
      }
      else
      {
	printf("%c%c%c%d) Off by %dm - %s",
	       22,32+report_line,32+14,
	       point_num+1,
	       player_inaccuracy,
	       rating
	  );
      }
#endif

      bit_beepfx(BEEPFX_ITEM_2);

      report_line++;
    }

    /* Actual round_score is an average of the accuracy for the cuts */
    round_score = round_score / num_cuts_allowed;
  }

  report_line++;
  printf("%c%c%cYour score this log: %d",22,32+report_line,32+21, round_score);

  sleep(2);

  /* Add this round's score to the overall score */
  *score += round_score;

  return game_over;
}

void main( void )
{
  im2_Init((void*)0xd300);
  memset((void*)0xd300, 0xd4, 257);
  bpoke((void*)0xd4d4, 195);
  wpoke((void*)0xd4d5, (uint16_t)myisr);
   
  uint16_t high_score = 0;

  while(1)
  {
    uint16_t score = 0;

    zx_border( INK_GREEN );

    clg();
    memset((void*)22528, PAPER_WHITE+INK_BLACK, 768);

    printf("%c%c%cv1.0",22,32+0,32+0);
    printf("%c%c%cLog chopper! Chop logs at the markers!",22,32+6,32+13);
    printf("%c%c%cActivate the chopper by pressing SPACE at the correct time",22,32+8,32+3);
    printf("%c%c%cStart your shift by pressing SPACE!",22,32+17,32+15);

    in_WaitForKey();

    uint8_t game_over = 0;
    for( uint8_t round_num = 0; round_num < NUM_ROUNDS; round_num++ )
    {
      /* Skip those marked as inactive, testing takes ages otherwise! */
      if( ! round[round_num].active )
	continue;

      clg();
      memset((void*)22528, PAPER_WHITE+INK_BLACK, 768);

      /* Print log number top left */
      printf("%c%c%cLog %d of %d",22,32+0,32+0, round_num+1, NUM_ROUNDS);

      /* Print current score at the top */
      printf("%c%c%cScore: %d",22,32+0,32+26, score);

      /* Print high score at the bottom */
      printf("%c%c%cHigh score: %d",22,32+23,32+24, high_score);

      /* Non zero return here means game over */
      game_over = play_round( round_num, &score );

      /* Update current score at the top */
      printf("%c%c%cScore: %d",22,32+0,32+26, score);

      if( score > high_score )
      {
	high_score = score;
	printf("%c%c%cHigh score: %d",22,32+23,32+24, high_score);
      }

      if( game_over )
      {
	bit_beepfx(BEEPFX_POWER_OFF);

	printf("%c%c%cGame over! Press SPACE to try again",22,32+20,32+15);

	in_WaitForKey();
	break;
      }
    }

    /* Do we have a winner? */
    if( ! game_over )
    {
      /* Game completed! */
      printf("%c%c%cGame complete! Well done!",22,32+20,32+19);
      in_WaitForKey();
      in_WaitForNoKey();
    }

  }

}
