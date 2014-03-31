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

struct particle_info_t {
  
  int time_to_live_frames;

  int column_idx;
  int row_idx;

  float column_fractional_idx;
  float row_fractional_idx;

  float velocity_columns_per_frame;
  float velocity_rows_per_frame;
};

struct game_physics_t {
  float impulse_velocity_rows_per_frame;
  float gravity_rows_per_frame_sq;
  float pipe_velocity_columns_per_frame;

  useconds_t frame_interval;
  float frame_interval_sec;
};

struct score_t {

  int current;
  int best;
};

const int number_of_rows = 30;
const int number_of_columns = 80;

const int bird_width_columns = 3;
const char* bird = "%@>";

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
  ",,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,";

const char* bg_floor =
  "////////////////////////////////////////////////////////////////////////////////\n"
  "////////////////////////////////////////////////////////////////////////////////";

const int floor_height = 2;

const int pipe_width_columns = 8;
const int pipe_end_height_rows = 2;

const int pipe_gap_height_rows = 7;

const char* lower_pipe_end         = "[[____]]";
const char* pipe_middle            = "|[    ]|";
const char* upper_pipe_penultimate = "|[____]|";
const char* upper_pipe_end         = "[[    ]]";

void initialize_physics( struct game_physics_t* );

enum game_state_e {
  TITLE_SCREEN,
  PLAY,
  BOOM
};

int title_screen( int flap, const struct score_t* );
int initialize_game( struct bird_info_t*, 
		     struct pipe_info_t*, 
		     int number_of_pipes,
		     struct score_t* );

int play( int flap, 
	  struct bird_info_t*, 
	  struct pipe_info_t*, 
	  int number_of_pipes, 
	  struct score_t* score, 
	  const struct game_physics_t* );

int boom( struct particle_info_t*, 
	  int number_of_particles, 
	  const struct pipe_info_t*, 
	  int number_of_pipes,
	  const struct game_physics_t*,
	  const struct score_t* );

void initialize_pipe( struct pipe_info_t*, int column_idx );
void draw_pipe( const struct pipe_info_t* );
void update_pipe( struct pipe_info_t*, const struct game_physics_t* );

void initialize_bird( struct bird_info_t*, 
		      int row_idx,
		      int column_idx );

void draw_bird( const struct bird_info_t* );
void update_bird( struct bird_info_t*, 
		  int flap, 
		  const struct game_physics_t* );

void initialize_particles( struct particle_info_t*,
			   int number_of_particles,
			   const struct game_physics_t*,
			   const struct bird_info_t* );

void draw_particles( struct particle_info_t*,
		     int number_of_particles );

void update_particles( struct particle_info_t*,
		       int number_of_particles,
		       const struct game_physics_t* );

enum collision_result_e {
  NONE,
  COLLISION,
  SCORE };

enum collision_result_e check_for_collisions( const struct bird_info_t*,
					      const struct pipe_info_t*,
					      int number_of_pipes );

int main()
{
  enum game_state_e game_state = TITLE_SCREEN;

  struct score_t score = { 0, 0 };

  useconds_t render_time = 0;

  struct timeval now = { 0, 0 }; 
  struct timeval then = { 0, 0 };

  struct game_physics_t game_physics;

  struct bird_info_t bird_info;

  const size_t number_of_pipes = 2;
  struct pipe_info_t pipes[number_of_pipes];

  const size_t number_of_particles = 40;
  struct particle_info_t particles[number_of_particles];

  int keep_looping = 1;
  int flap = 0;

  

  initialize_physics( &game_physics );

  initscr();
  nodelay(stdscr, TRUE);
  curs_set(0);
  start_color();

  init_pair( 1, COLOR_YELLOW, COLOR_BLACK );
  init_pair( 2, COLOR_WHITE, COLOR_GREEN );
  init_pair( 3, COLOR_RED, COLOR_BLACK );
  init_pair( 4, COLOR_CYAN, COLOR_BLACK );
  init_pair( 5, COLOR_YELLOW, COLOR_GREEN );

  gettimeofday( &then, NULL );
  
  while ( keep_looping ) {

    gettimeofday( &now, NULL );

    clear();

    attron(COLOR_PAIR(4));
    mvprintw( 0, 0, background );
    attroff(COLOR_PAIR(4));

    attron(COLOR_PAIR(5));
    mvprintw( number_of_rows - floor_height, 0, bg_floor );
    attroff(COLOR_PAIR(5));

    

    switch ( game_state ){
    case TITLE_SCREEN:

      if ( title_screen( flap, &score ) ){
	initialize_game( &bird_info, 
			 pipes, 
			 number_of_pipes, 
			 &score );
	game_state = PLAY;
      }
      break;

    case PLAY:
      if ( play( flap, 
		 &bird_info, 
		 pipes, 
		 number_of_pipes, 
		 &score,
		 &game_physics ) ){

	initialize_particles( particles,
			      number_of_particles,
			      &game_physics,
			      &bird_info );
	game_state = BOOM;
      }
      break;

    case BOOM:

      if ( boom( particles, 
		 number_of_particles, 
		 pipes, 
		 number_of_pipes,
		 &game_physics,
		 &score ) ){
	game_state = TITLE_SCREEN;
      }
    }

    refresh();

    render_time = 
      (now.tv_sec - then.tv_sec) * 1000000 + (now.tv_usec - then.tv_usec);

    if ( render_time < game_physics.frame_interval )
    {
      usleep( game_physics.frame_interval - render_time );
    }

    then = now;

    int ch = getch();

    keep_looping = ( 'q' != ch );
    flap         = ( ' ' == ch );

  }

  endwin();

  return 0;
}

void initialize_physics( struct game_physics_t* game_physics ) {

  const float impulse_rows_per_sec = -7.5f;
  const float gravity_rows_per_sec_sq = 0.75f;
  const float pipe_velocity_columns_per_sec = -10.0f;

  game_physics->frame_interval = 100000;

  game_physics->frame_interval_sec = game_physics->frame_interval*0.000001f;

  game_physics->impulse_velocity_rows_per_frame = 
    impulse_rows_per_sec * game_physics->frame_interval_sec;

  game_physics->gravity_rows_per_frame_sq = 
    gravity_rows_per_sec_sq * game_physics->frame_interval_sec;

  game_physics->pipe_velocity_columns_per_frame = 
    pipe_velocity_columns_per_sec * game_physics->frame_interval_sec;
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

  attron(COLOR_PAIR(2));
    

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

  attroff(COLOR_PAIR(2));

}

void update_pipe( struct pipe_info_t* pipe_info, 
		  const struct game_physics_t* game_physics ){

  pipe_info->left_column_fractional_idx += 
    game_physics->pipe_velocity_columns_per_frame;
  pipe_info->right_column_fractional_idx += 
    game_physics->pipe_velocity_columns_per_frame;

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

  attron(COLOR_PAIR(1));
  mvprintw( bird_info->row_idx, 
	    bird_info->left_column_idx, 
	    bird );
  attroff(COLOR_PAIR(1));

}

void update_bird( struct bird_info_t* bird_info, 
		  int flap,
		  const struct game_physics_t* game_physics ) {

  if ( flap ) {
    bird_info->velocity_rows_per_frame = 
      game_physics->impulse_velocity_rows_per_frame;
  }
  else {
    bird_info->velocity_rows_per_frame += 
      game_physics->gravity_rows_per_frame_sq;
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

void initialize_particles( struct particle_info_t* particles,
			   int number_of_particles,
			   const struct game_physics_t* game_physics,
			   const struct bird_info_t* bird_info ) {

  int particle_idx = 0;
  for ( particle_idx = 0;
	particle_idx < number_of_particles;
	++particle_idx ) {

    struct particle_info_t* particle = &particles[particle_idx];

    /* 1 to 2 seconds */
    float time_to_live_sec = 0.1f * (rand() % 10) + 1.0f;

    /* 1 to 10 columns per second */
    float velocity_columns_per_sec = 0.1f * (rand() % 90) + 1.0f;
    if ( rand() % 2 ) {
      velocity_columns_per_sec *= -1.0f;
    }

    /* 1 to 10 rows per second */
    float velocity_rows_per_sec = -0.1f * (rand() % 90) + 1.0f;

    particle->time_to_live_frames = 
      (int)((time_to_live_sec / game_physics->frame_interval_sec) + 0.5f); 
  
    particle->velocity_columns_per_frame =
      (velocity_columns_per_sec * game_physics->frame_interval_sec);

    particle->velocity_rows_per_frame = 
      (velocity_rows_per_sec * game_physics->frame_interval_sec);

    particle->column_fractional_idx = 
      particle->column_idx = 
      bird_info->left_column_idx;

    particle->row_fractional_idx = 
      particle->row_idx = 
      bird_info->row_idx;
  }

  /* at least one particle always lives for 2 seconds so the *boom* is */
  /* of consistent duration */
  particles[0].time_to_live_frames = 
    (int)((2.0 / game_physics->frame_interval_sec) + 0.5f);
}

void draw_particle( const struct particle_info_t* particle ) {

  if ( 0 < particle->time_to_live_frames && 
       0 < particle->row_idx && particle->row_idx < number_of_rows &&
       0 < particle->column_idx && particle->column_idx < number_of_columns ) {

    mvprintw( particle->row_idx,
	      particle->column_idx,
	      "X" );
  }
}

void update_particle( struct particle_info_t* particle,
		      const struct game_physics_t* game_physics ) {

  if ( 0 < particle->time_to_live_frames ) {

    particle->time_to_live_frames -= 1;

    particle->column_fractional_idx += 
      particle->velocity_columns_per_frame;

    particle->velocity_rows_per_frame += 
      game_physics->gravity_rows_per_frame_sq;

    particle->row_fractional_idx += 
      particle->velocity_rows_per_frame;
    
    particle->column_idx = 
      (int)(particle->column_fractional_idx + 0.5f);

    particle->row_idx = 
      (int)(particle->row_fractional_idx + 0.5f);
  }
}

int title_screen( int flap, const struct score_t* score ) {

  mvprintw(  5, 32, "FLOPPY TERM" );
  mvprintw(  9, 30, "Last Score: %3d", score->current );
  mvprintw( 11, 30, "Best Score: %3d", score->best );

  mvprintw( 20, 20, "Press SPACEBAR to flap and to start game" );
  mvprintw( 22, 20, "Press 'q' to quit" );

  return (flap != 0);
}

int initialize_game( struct bird_info_t* bird_info, 
		     struct pipe_info_t* pipes, 
		     int number_of_pipes,
		     struct score_t* score) {
  
  int result = 0;

  initialize_bird( bird_info, 10, 20 );

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

  score->current = 0;

  return result;
}

int play( int flap, 
	  struct bird_info_t* bird_info, 
	  struct pipe_info_t* pipes, 
	  int number_of_pipes,
	  struct score_t* score,
	  const struct game_physics_t* game_physics ) {

  int result = 0;
  
  int pipe_idx = 0;
  for ( pipe_idx = 0;
	pipe_idx < number_of_pipes;
	++pipe_idx ) {
    
    draw_pipe( &pipes[pipe_idx] );
    update_pipe( &pipes[pipe_idx], game_physics );
  }

  draw_bird( bird_info );
  update_bird( bird_info, 
	       flap, 
	       game_physics );
  
  enum collision_result_e 
    collision_result = check_for_collisions( bird_info, 
					     pipes, 
					     number_of_pipes );
  
  switch ( collision_result ) {
    
  case COLLISION: 
    result = 1;
    break;
    
  case SCORE: 
    ++(score->current); 
    if ( score->best < score->current ) {
      score->best = score->current;
    }
    
  default:
    break;
  }

  mvprintw( number_of_rows-1, 1, " score: %d ", score->current );

  return result;
}

int boom( struct particle_info_t* particles, 
	  int number_of_particles,
	  const struct pipe_info_t* pipes, 
	  int number_of_pipes,
	  const struct game_physics_t* game_physics,
	  const struct score_t* score ) {

  int result = 1;

  int pipe_idx = 0;
  for ( pipe_idx = 0;
	pipe_idx < number_of_pipes;
	++pipe_idx ) {
    draw_pipe( &pipes[pipe_idx] );
  }


  attron(COLOR_PAIR(3));
  int particle_idx = 0;
  for ( particle_idx = 0;
	particle_idx < number_of_particles;
	++particle_idx ) {

    draw_particle( &particles[particle_idx] );
    update_particle( &particles[particle_idx],
		     game_physics );

    if ( 0 < particles[particle_idx].time_to_live_frames ) {
      result = 0;
    }
  }
  attroff(COLOR_PAIR(3));

  mvprintw( number_of_rows-1, 1, " score: %d ", score->current );

  return result;
}

