#include <curses.h>
#include <unistd.h>
#include <sys/time.h>
#include <string.h>

struct pipe_info_t {
  
  int opening_top_row_idx;
  int opening_bottom_row_idx;

  int left_column_idx;
  int right_column_idx;
  
  int right_column_idx_last_frame;

  float left_column_fractional_idx;
  float right_column_fractional_idx;
};

struct bird_info_t {

  int row_idx;
  
  float row_fractional_idx;
  
  int left_column_idx;
  int right_column_idx;

  float velocity_rows_per_frame;
};

const int number_of_rows = 30;
const int number_of_columns = 80;

const int bird_width_columns = 3;
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

const char* lower_pipe_end         = "[[____]]";
const char* pipe_middle            = "|[    ]|";
const char* upper_pipe_penultimate = "|[____]|";
const char* upper_pipe_end         = "[[    ]]";

void initialize_pipe( struct pipe_info_t*, int column_idx );
void draw_pipe( const struct pipe_info_t* );
void update_pipe( struct pipe_info_t*, float pipe_velocity_columns_per_frame );

enum game_state_e {
  TITLE_SCREEN,
  INITIALIZE,
  PLAY,
  BOOM
};

void title_screen( int flap );
void initialize( struct bird_info_t* );
void play( struct bird_info_t*, struct pipe_info_t*, int flap );
void boom( struct  bird_info_t*, struct pipe_info_t* );

void initialize_bird( struct bird_info_t*, 
		      int row_idx,
		      int column_idx );

void draw_bird( const struct bird_info_t* );
void update_bird( struct bird_info_t*, 
		  int flap, 
		  float impulse_velocity_rows_per_frame,
		  float gravity_rows_per_frame_sq );

enum collision_result_e {
  NONE,
  COLLISION,
  SCORE };

enum collision_result_e check_for_collisions( const struct bird_info_t*,
					      const struct pipe_info_t*,
					      int number_of_pipes );

int main()
{
  int current_score = 0;
  int best_score = 0;

  useconds_t render_time = 0;
  const useconds_t frame_interval = 80000;
  const float frame_interval_sec = frame_interval*0.000001f;

  struct timeval now = { 0, 0 }; 
  struct timeval then = { 0, 0 };

  float bird_velocity_rows_per_frame = 0.0f;

  const float impulse_rows_per_sec = -7.5f;
  const float impulse_rows_per_frame = 
    impulse_rows_per_sec * frame_interval_sec;

  const float gravity_rows_per_sec_sq = 0.75f;
  const float gravity_rows_per_frame_sq = 
    gravity_rows_per_sec_sq * frame_interval_sec;

  const float pipe_velocity_columns_per_sec = -10.0f;
  const float pipe_velocity_columns_per_frame = 
    pipe_velocity_columns_per_sec * frame_interval_sec;

  struct bird_info_t bird_info;

  int keep_looping = 1;
  int flap = 0;

  const size_t number_of_pipes = 2;
  struct pipe_info_t pipes[number_of_pipes];

  const size_t initial_pipe_column = number_of_columns;
  const size_t pipe_spacing_columns = ((number_of_columns/2) + 
				       (pipe_width_columns/2));

  size_t pipe_idx = 0;
  for ( pipe_idx = 0;
	pipe_idx < number_of_pipes;
	++pipe_idx ) {

    initialize_pipe( &pipes[pipe_idx], 
		     initial_pipe_column + (pipe_idx*pipe_spacing_columns ) );
  }

  initialize_bird( &bird_info, 10, 20 );

  initscr();
  nodelay(stdscr, TRUE);

  curs_set(0);

  gettimeofday( &then, NULL );
  
  while ( keep_looping ) {

    gettimeofday( &now, NULL );

    clear();

    mvprintw( 0, 0, background );

    for ( pipe_idx = 0;
	  pipe_idx < number_of_pipes;
	  ++pipe_idx ) {

      draw_pipe( &pipes[pipe_idx] );
      update_pipe( &pipes[pipe_idx], pipe_velocity_columns_per_frame );
    }
      

    draw_bird( &bird_info );
    update_bird( &bird_info, 
		 flap, 
		 impulse_rows_per_frame, 
		 gravity_rows_per_frame_sq );

    enum collision_result_e 
      collision_result = check_for_collisions( &bird_info, 
					       pipes, 
					       number_of_pipes );
    
    switch ( collision_result ) {

    case COLLISION: 
      current_score = 0; 
      break;

    case SCORE: 
      ++current_score; 
      if ( best_score < current_score ) {
	best_score = current_score;
      }

    default:
      break;
    }

    mvprintw( number_of_rows-1, 1, "current: %d  best: %d", current_score, best_score );

    refresh();

    render_time = 
      (now.tv_sec - then.tv_sec) * 1000000 + (now.tv_usec - then.tv_usec);

    if ( render_time < frame_interval )
    {
      usleep( frame_interval - render_time );
    }

    then = now;

    int ch = getch();

    keep_looping = ( 'q' != ch );
    flap         = ( ' ' == ch );

  }

  endwin();

  return 0;
}

void initialize_pipe( struct pipe_info_t* pipe_info, int column_idx ) {

  pipe_info->left_column_idx = column_idx;
  pipe_info->left_column_fractional_idx = column_idx;

  pipe_info->right_column_idx = (column_idx + pipe_width_columns);
  pipe_info->right_column_fractional_idx = (column_idx + pipe_width_columns);

  pipe_info->right_column_idx_last_frame = pipe_info->right_column_idx;

  pipe_info->opening_top_row_idx = 
    (rand() % (number_of_rows - 
	       floor_height - 
	       pipe_gap_height_rows - 
	       2*pipe_end_height_rows )) + pipe_end_height_rows;

  pipe_info->opening_bottom_row_idx = (pipe_info->opening_top_row_idx + 
				       pipe_gap_height_rows);
}

void draw_pipe( const struct pipe_info_t* pipe_info ) {

  /* short circuit if off screen */
  if ( number_of_columns < pipe_info->left_column_idx )
  {
    return;
  }

  char pipe_middle_cp[ pipe_width_columns+1 ];
  char upper_pipe_end_cp[ pipe_width_columns+1 ];
  char upper_pipe_penultimate_cp[ pipe_width_columns+1 ];
  char lower_pipe_end_cp[ pipe_width_columns+1 ];

  int cp_idx_start = 0;
  if ( pipe_info->left_column_idx < 0 ){
    cp_idx_start = abs( pipe_info->left_column_idx );
  }

  strncpy( pipe_middle_cp, pipe_middle+cp_idx_start, pipe_width_columns );
  strncpy( lower_pipe_end_cp, lower_pipe_end+cp_idx_start, pipe_width_columns );
  strncpy( upper_pipe_end_cp, upper_pipe_end+cp_idx_start, pipe_width_columns );
  strncpy( upper_pipe_penultimate_cp, upper_pipe_penultimate+cp_idx_start, pipe_width_columns );

  int null_idx = pipe_width_columns;
  if ( number_of_columns < pipe_info->right_column_idx ) {
    null_idx = (number_of_columns - pipe_info->left_column_idx);
  }
  
  pipe_middle_cp[null_idx] = '\0';
  lower_pipe_end_cp[null_idx] = '\0';
  upper_pipe_end_cp[null_idx] = '\0';
  upper_pipe_penultimate_cp[null_idx] = '\0';

  int col_idx = pipe_info->left_column_idx;
  if ( col_idx < 0 ) {
    col_idx = 0;
  }
    

  int row_idx = 0;
  for ( row_idx = 0;
	row_idx < (number_of_rows - floor_height);
	++row_idx ) {
    
    if ( row_idx <= pipe_info->opening_top_row_idx ) {

      if ( row_idx < pipe_info->opening_top_row_idx-1 ){ 
	mvprintw( row_idx, col_idx, pipe_middle_cp );
      }
      else if ( row_idx < pipe_info->opening_top_row_idx ){
	mvprintw( row_idx, col_idx, upper_pipe_penultimate_cp );
      }
      else { 
	mvprintw( row_idx, col_idx, upper_pipe_end_cp );
      }
    }
    else if ( pipe_info->opening_bottom_row_idx <= row_idx ) {
      
      if ( row_idx == pipe_info->opening_bottom_row_idx ){
	mvprintw( row_idx, col_idx, lower_pipe_end_cp );
      }
      else{
	mvprintw( row_idx, col_idx, pipe_middle_cp );
      }
    }
  }
}

void update_pipe( struct pipe_info_t* pipe_info, 
		  float pipe_velocity_columns_per_frame ){

  pipe_info->left_column_fractional_idx += pipe_velocity_columns_per_frame;
  pipe_info->right_column_fractional_idx += pipe_velocity_columns_per_frame;

  pipe_info->left_column_idx = 
    (int)(pipe_info->left_column_fractional_idx+0.5f);

  pipe_info->right_column_idx_last_frame = pipe_info->right_column_idx;

  pipe_info->right_column_idx = 
    (int)(pipe_info->right_column_fractional_idx+0.5f);

  if ( pipe_info->right_column_idx < 0 ){
    initialize_pipe( pipe_info, number_of_columns );
  }
}

void initialize_bird( struct bird_info_t* bird_info,
		      int row_idx,
		      int column_idx ) {

  bird_info->row_idx = row_idx;
  bird_info->row_fractional_idx = row_idx;

  bird_info->left_column_idx = column_idx;
  bird_info->right_column_idx = column_idx + bird_width_columns;

  bird_info->velocity_rows_per_frame = 0.0f;
}

void draw_bird( const struct bird_info_t* bird_info ) {

  mvprintw( bird_info->row_idx, 
	    bird_info->left_column_idx, 
	    bird );
}

void update_bird( struct bird_info_t* bird_info, 
		  int flap,
		  float impulse_velocity_rows_per_frame,
		  float gravity_rows_per_frame_sq ) {

  if ( flap ) {
    bird_info->velocity_rows_per_frame = impulse_velocity_rows_per_frame;
  }
  else {
    bird_info->velocity_rows_per_frame += gravity_rows_per_frame_sq;
  }

  bird_info->row_fractional_idx += bird_info->velocity_rows_per_frame;

  bird_info->row_idx = (int)(bird_info->row_fractional_idx + 0.5f);

  if ( bird_info->row_idx < 0 ) {
    bird_info->row_idx = 0;
  }
  
  if ( number_of_rows - floor_height < bird_info->row_idx ) {
    bird_info->row_idx = number_of_rows - floor_height;
    bird_info->row_fractional_idx = bird_info->row_idx;
  }
}

enum collision_result_e check_for_collisions( const struct bird_info_t* bird_info,
					      const struct pipe_info_t* pipe_info,
					      int number_of_pipes ) {

  enum collision_result_e result = NONE;

  int pipe_idx = 0;

  for ( pipe_idx = 0;
	pipe_idx < number_of_pipes;
	++pipe_idx ){

    if ( pipe_info[pipe_idx].left_column_idx <= bird_info->right_column_idx &&
	 bird_info->left_column_idx <= pipe_info[pipe_idx].right_column_idx &&
	 ( bird_info->row_idx <= pipe_info[pipe_idx].opening_top_row_idx ||
	   pipe_info[pipe_idx].opening_bottom_row_idx <= bird_info->row_idx ) ) {
      result = COLLISION;
    }
    else if ( ( number_of_rows - floor_height ) <= bird_info->row_idx ) {
      result = COLLISION;
    }
    else if ( ( bird_info->left_column_idx == 
		pipe_info[pipe_idx].right_column_idx_last_frame ) &&
	      ( pipe_info[pipe_idx].right_column_idx < 
		pipe_info[pipe_idx].right_column_idx_last_frame ) ) {
      result = SCORE;
    }
  }

  return result;
}
