#include <curses.h>
#include <unistd.h>
#include <sys/time.h>
#include <string.h>

struct pipe_info_t {
  
  int opening_top_row_idx;
  int opening_bottom_row_idx;
  
  int left_column_idx;
  int right_column_idx;

  int upper_pipe_char_offset;
  int lower_pipe_char_offset;

  int upper_pipe_middle_row_idx;
  int upper_pipe_end_row_idx;
  int lower_pipe_end_row_idx;
  int lower_pipe_middle_row_idx;
};

const size_t number_of_rows = 30;
const size_t number_of_columns = 80;

const char* bird = "@@@";

const char* background = 
  ". . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . \n"
  " . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .\n"
  ". . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . \n"
  " . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .\n"
  ". . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . \n"
  " . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .\n"
  ". . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . \n"
  " . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .\n"
  ". . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . \n"
  " . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .\n"
  ". . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . \n"
  " . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .\n"
  ". . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . \n"
  " . . . . . . . : : . . . . . . . . . . . . . : : . . . . . . . . . . . . . . . .\n"
  ". . . : : . : : : : : . . . . . . : : : . : : : : : . . . . . . . : : : . . : : \n"
  " . : : : : : : : : : : : . . . : : : : : : : : : : : : . . . . : : : : : : : : :\n"
  ": : : : : : : : : : : : : : : : : : : : : : : : : : : : : : : : : : : : : : : : \n"
  " : : : : : : : : : : : : : : : : : : : : : : : : : : : : : : : : : : : : : : : :\n"
  ": : : : : : : : : : : : : : : : : : : : : : : : : : : : : : : : : : : : : : : : \n"
  " : : : : : : : : : : : : : : : : : : : : : : : : : : : : : : : : : : : : : : : :\n"
  ": : : : : : : : : : : : : : : : : ,,,,: : : : : : : : : : :,,,,,,,: : : : : : : \n"
  " : : : : ,,,,,,, : : : : : : :,,,,,,,,,,,: : : : : : : ,,,,,,,,,,,,,,: : : : : :\n"
  ": : : ,,,,,,,,,,,,, : : : :,,,,,,,,,,,,,,,,,: : : : :,,,,,,,,,,,,,,,,,,,: : : ,,\n"
  " : ,,,,,,,,,,,,,,,,,,, :,,,,,,,,,,,,,,,,,,,,,,,: ,,,,,,,,,,,,,,,,,,,,,,,,,,: ,,,\n"
  ",,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,\n"
  ",,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,\n"
  ",,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,\n"
  ",,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,\n"
  "################################################################################\n"
  "################################################################################";

const int floor_height = 2;

const int pipe_width_columns = 8;
const int pipe_end_height_rows = 2;

const int pipe_gap_height_rows = 7;

const char* upper_pipe_end = "|[____]|\n"
                             "[[    ]]";


const char* lower_pipe_end = "[[____]]\n"
                             "|[    ]|";

const char* pipe_middle = 
  "|[    ]|\n"    /* row 0 offset 0 */   
  "|[    ]|\n"    
  "|[    ]|\n"
  "|[    ]|\n"
  "|[    ]|\n"    
  "|[    ]|\n"    /* row 5 offset 45 */
  "|[    ]|\n"
  "|[    ]|\n"
  "|[    ]|\n"
  "|[    ]|\n"    
  "|[    ]|\n"    /* row 10 offset 90 */
  "|[    ]|\n"
  "|[    ]|\n"
  "|[    ]|\n"
  "|[    ]|\n"
  "|[    ]|\n"    /* row 15 offset 135 */
  "|[    ]|\n"
  "|[    ]|\n"
  "|[    ]|\n"
  "|[    ]|\n"
  "|[    ]|\n"    /* row 20 offset 180 */
  "|[    ]|\n"
  "|[    ]|\n"
  "|[    ]|\n"
  "|[    ]|\n"
  "|[    ]|\n"    /* row 25 offset 225 */
  "|[    ]|\n"
  "|[    ]|\n"
  "|[    ]|\n"
  "|[    ]|\n"
  "|[    ]|";     /* row 30 offset 270 */

void initialize_pipe( struct pipe_info_t*, int column_idx );

int main()
{
  useconds_t render_time = 0;
  const useconds_t frame_interval = 80000;

  struct timeval now = { 0, 0 }; 
  struct timeval then = { 0, 0 };

  float bird_row_fractional_idx = 10.0f;
  const int bird_column_idx = 20;
  float bird_velocity_rows_per_frame = 0.0f;

  const float impulse_rows_per_sec = -7.5f;
  const float impulse_rows_per_frame = 
    impulse_rows_per_sec * (0.000001f*frame_interval);

  const float gravity_rows_per_sec_sq = 0.75f;
  const float gravity_rows_per_frame_sq = 
    gravity_rows_per_sec_sq * (0.000001f*frame_interval);

  size_t row_idx = 0;

  int keep_looping = 1;
  int flap = 0;

  struct pipe_info_t pipes[3];

  initialize_pipe( &pipes[0], 60 );
  initialize_pipe( &pipes[1], 120 );
  initialize_pipe( &pipes[2], 160 );

  initscr();
  nodelay(stdscr, TRUE);

  curs_set(0);

  gettimeofday( &then, NULL );
  
  while ( keep_looping ) {

    gettimeofday( &now, NULL );

    if ( flap ) {
      
      bird_velocity_rows_per_frame = impulse_rows_per_frame;
      flap = 0;
    }
    else {
      bird_velocity_rows_per_frame += gravity_rows_per_frame_sq;
    }

    bird_row_fractional_idx += bird_velocity_rows_per_frame;

    clear();

    mvprintw( 0, 0, background );

    mvprintw( (int)(bird_row_fractional_idx + 0.5f), 
	      bird_column_idx, 
	      bird );

    if ( 0 < pipes[0].right_column_idx ) {
      
      mvprintw( pipes[0].upper_pipe_middle_row_idx,
		pipes[0].left_column_idx,
		&pipe_middle[ pipes[0].upper_pipe_char_offset ]);

      mvprintw( pipes[0].upper_pipe_end_row_idx,
		pipes[0].left_column_idx,
		upper_pipe_end );
    }


    refresh();

    render_time = 
      (now.tv_sec - then.tv_sec) * 1000000 + (now.tv_usec - then.tv_usec);

    if ( render_time < frame_interval )
    {
      usleep( frame_interval - render_time );
    }

    then = now;

    int c = getch();

    keep_looping = ( 'q' != c );
    flap         = ( ' ' == c );
  }

  endwin();

  return 0;
}

void initialize_pipe( struct pipe_info_t* pipe_info, int column_idx ) {

  pipe_info->left_column_idx = column_idx;
  pipe_info->right_column_idx = column_idx + pipe_width_columns;

  pipe_info->opening_top_row_idx = 
    (rand() % (number_of_rows - 
	       floor_height - 
	       pipe_gap_height_rows - 
	       2*pipe_end_height_rows )) + pipe_end_height_rows;

  pipe_info->opening_bottom_row_idx = (pipe_info->opening_top_row_idx + 
				       pipe_gap_height_rows);

  pipe_info->upper_pipe_middle_row_idx = 0;
  pipe_info->upper_pipe_end_row_idx = (pipe_info->opening_top_row_idx -
				       pipe_end_height_rows);

  pipe_info->lower_pipe_end_row_idx = pipe_info->opening_bottom_row_idx;
  pipe_info->lower_pipe_middle_row_idx = (pipe_info->opening_bottom_row_idx +
					  pipe_end_height_rows);

  int number_of_pipe_chars_per_row = (pipe_width_columns+1);

  size_t pipe_middle_str_len = strnlen( pipe_middle, 1024 );
  size_t number_of_pipe_middle_rows = 
    (pipe_middle_str_len+1) / number_of_pipe_chars_per_row;

  pipe_info->upper_pipe_char_offset = 
    ( number_of_pipe_middle_rows - 
      pipe_info->upper_pipe_end_row_idx ) * number_of_pipe_chars_per_row;

  pipe_info->lower_pipe_char_offset = 
    ( number_of_rows - 
      floor_height - 
      pipe_info->lower_pipe_middle_row_idx ) * number_of_pipe_chars_per_row;
}
