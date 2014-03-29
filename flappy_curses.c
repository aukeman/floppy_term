#include <curses.h>
#include <unistd.h>
#include <sys/time.h>
#include <string.h>

int main()
{
  useconds_t render_time = 0;
  const useconds_t frame_interval = 80000;

  struct timeval now = { 0, 0 }; 
  struct timeval then = { 0, 0 };

  const size_t number_of_rows = 30;
  const size_t number_of_columns = 80;

  float bird_row_fractional_idx = 10.0f;
  const int bird_column_idx = 20;
  float bird_velocity_rows_per_frame = 0.0f;

  const float impulse_rows_per_sec = -7.5f;
  const float impulse_rows_per_frame = 
    impulse_rows_per_sec * (0.000001f*frame_interval);

  const float gravity_rows_per_sec_sq = 0.75f;
  const float gravity_rows_per_frame_sq = 
    gravity_rows_per_sec_sq * (0.000001f*frame_interval);

  char buffers[number_of_rows][number_of_columns+1];

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

  const char* pipe =
    "[[____]]\n"
    "|[    ]|\n"
    "|[    ]|\n"
    "|[    ]|\n"
    "|[    ]|\n"
    "|[    ]|\n"
    "|[    ]|\n"
    "|[    ]|\n"
    "|[    ]|\n"
    "|[    ]|\n"
    "|[    ]|\n"
    "|[    ]|\n"
    "|[    ]|\n"
    "|[    ]|\n"
    "|[    ]|\n"
    "|[    ]|\n"
    "|[    ]|\n"
    "|[    ]|\n"
    "|[    ]|\n"
    "|[    ]|\n"
    "|[    ]|\n"
    "|[    ]|\n"
    "|[    ]|\n"
    "|[    ]|\n"
    "|[    ]|\n"
    "|[    ]|\n"
    "|[    ]|\n"
    "|[    ]|\n"
    "|[    ]|\n"
    "|[____]|\n"
    "[[    ]]";


  size_t row_idx = 0;

  int keep_looping = 1;
  int flap = 0;

  for ( row_idx = 0;
	row_idx < number_of_rows;
	++row_idx ) {
    buffers[row_idx][number_of_columns] = '\0';
  }

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

    /* for ( row_idx = 0; */
    /* 	  row_idx < number_of_rows; */
    /* 	  ++row_idx ) { */
    /*   int pipe_col = 0; */

    /*   memset( buffers[row_idx], bg_char, sizeof(char)*number_of_columns ); */

    /*   /\* for ( pipe_col = col; *\/ */
    /*   /\* 	    (pipe_col < col+4 && *\/ */
    /*   /\* 	     pipe_col < number_of_columns); *\/ */
    /*   /\* 	    ++pipe_col ) { *\/ */

    /*   /\* 	buffers[row_idx][pipe_col] = pipe_char; *\/ */
    /*   /\* } *\/ */
	


    /*   /\* buffers[row_idx][col] = pipe_char; *\/ */
    /* } */

    /* for ( row_idx = 0; */
    /* 	  row_idx < number_of_rows; */
    /* 	  ++row_idx ) { */

    /*   /\* printf( "%s\n", &buffers[row_idx][0] ); *\/ */
      
    /*   mvprintw(row_idx, 0, &buffers[row_idx][0]); */
    /* } */

    mvprintw( 0, 0, background );

    mvprintw( (int)(bird_row_fractional_idx + 0.5f), 
	      bird_column_idx, 
	      bird );

    mvprintw( 29, 1, "vel: %f  row_idx: %f", 
	      bird_velocity_rows_per_frame,
	      bird_row_fractional_idx );

    /* mvprintw(0, col, "Hello World !!!"); */
    refresh();

    render_time = 
      (now.tv_sec - then.tv_sec) * 1000000 + (now.tv_usec - then.tv_usec);

    if ( render_time < frame_interval )
    {
      usleep( frame_interval - render_time );
    }

    /* --col; */

    then = now;

    int c = getch();

    keep_looping = ( 'q' != c );
    flap =         ( ' ' == c );
  }

  endwin();

  return 0;
}
