#include "WaterMode.hpp"

#include "Sprite.hpp"
#include "DrawSprites.hpp"
#include "Load.hpp"
#include "data_path.hpp"
#include "gl_errors.hpp"
#include "MenuMode.hpp"
#include "Sound.hpp"
#include <random>

Sprite const *sprite_water_bottle = nullptr;
Sprite const *sprite_water = nullptr;
Sprite const *sprite_background = nullptr;
Sprite const *sprite_faucet = nullptr;

Load< SpriteAtlas > text_sprites( LoadTagDefault, []() -> SpriteAtlas const * {
	SpriteAtlas const *ret = new SpriteAtlas( data_path( "the-planet" ) );
	return ret;
} );

Load< SpriteAtlas > water_sprites(LoadTagDefault, []() -> SpriteAtlas const * {
	SpriteAtlas const *ret = new SpriteAtlas(data_path("water_sprites"));
	sprite_water_bottle = &ret->lookup( "bottle" );
	sprite_water = &ret->lookup( "square" );
	sprite_background = &ret->lookup( "background" );
	sprite_faucet = &ret->lookup( "faucet" );
	return ret;
});

Load< Sound::Sample > sound_faucet( LoadTagDefault, []() -> Sound::Sample * {
	return new Sound::Sample( data_path( "water-flow-looped.wav" ) );
} );

Load< Sound::Sample > sound_water_fill( LoadTagDefault, []() -> Sound::Sample * {
	return new Sound::Sample( data_path( "bottle-fill-start.wav" ) );
} );

Load< Sound::Sample > sound_water_spill( LoadTagDefault, []() -> Sound::Sample * {
	return new Sound::Sample( data_path( "bottle-fill-spill.wav" ) );
} );

WaterMode::WaterMode() {
}

WaterMode::~WaterMode() {
}

bool WaterMode::handle_event(SDL_Event const &evt, glm::uvec2 const &window_size) {
	if( evt.type == SDL_MOUSEMOTION ) {
		//convert mouse from window pixels to view pixels
		mouse_screen_pos = glm::vec2(
			view_max.x * evt.motion.x / window_size.x,
			view_max.y * ( window_size.y - evt.motion.y ) / window_size.y
		);

	}

	return false;
}

void WaterMode::update(float elapsed)
{
	bool prev_entering = is_water_entering_bottle;
	is_water_entering_bottle = std::abs( mouse_screen_pos.x - water_spawn_pos.x ) < 1.5f*water_radius 
								&& mouse_screen_pos.y + 40 < water_spawn_pos.y 
								&& !bottle_drop;

	if( is_water_entering_bottle )
	{
		if( prev_entering != is_water_entering_bottle )
		{
			filling_sound = Sound::play( *sound_water_fill, 1.0f, 0.0f, std::min(bottle_fill_time/bottle_fill_max,1.0f) );
		}

		bottle_fill_time += elapsed;
		if( bottle_fill_time > bottle_fill_max )
		{
			// Drop bottle ouch!
			static std::mt19937 mt;
			bottle_velocity = 400.0f * glm::vec2( ( mt() / float( mt.max() ) ) - 0.5f, ( mt() / float( mt.max() ) ) - 0.5f );
			bottle_pos = mouse_screen_pos;
			bottle_drop = true;
			Sound::play( *sound_water_spill );
		}
	}
	else
	{
		if( filling_sound && !filling_sound->stopped )
		{
			filling_sound->stop();
		}
	}

	if( !faucet_sound || ( filling_sound && filling_sound->stopped && faucet_sound->stopped ) )
	{
		faucet_sound = Sound::play( *sound_faucet );
	}
	
	if( faucet_sound && !faucet_sound->stopped && filling_sound && !filling_sound->stopped )
	{
		faucet_sound->stop();
	}

	if( bottle_drop )
	{
		bottle_velocity += elapsed * gravity;
		bottle_pos += elapsed * bottle_velocity;
		if( bottle_pos.y < respawn_height )
		{
			bottle_fill_time = 0.0f;
			bottle_drop = false;
		}
	}
	else if( mouse_screen_pos.x <= handing_off_border_size)
	{
		last_filled_percent = 100.0f * bottle_fill_time / bottle_fill_max;
		bottle_velocity = glm::vec2(-800.0f, 100.0f);
		bottle_pos = mouse_screen_pos;
		bottle_drop = true;
	}
	else if( mouse_screen_pos.x >= view_max.x - handing_off_border_size )
	{
		last_filled_percent = 100.0f * bottle_fill_time / bottle_fill_max;
		bottle_velocity = glm::vec2( 800.0f, 100.0f );
		bottle_pos = mouse_screen_pos;
		bottle_drop = true;
	}
}


void WaterMode::draw(glm::uvec2 const &drawable_size) {
	//clear the color buffer:
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	//use alpha blending:
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	//don't use the depth test:
	glDisable(GL_DEPTH_TEST);

	{ //use a DrawSprites to do the drawing:
		DrawSprites draw(*water_sprites, view_min, view_max, drawable_size, DrawSprites::AlignPixelPerfect);

		draw.draw( *sprite_background, glm::vec2(0,view_max.y));

		glm::vec2 water_min_pos = glm::vec2( water_spawn_pos.x - water_radius, water_spawn_pos.y );
		float water_end_y = is_water_entering_bottle ? mouse_screen_pos.y : 0;
		float shake = is_water_entering_bottle ? 2.0f : 0.0f;
		glm::vec2 water_max_pos = glm::vec2( water_spawn_pos.x + water_radius, water_end_y );
		glm::u8vec4 water_tint = glm::u8vec4(135,184,221,255);
		draw.draw_absolute( *sprite_water, water_min_pos, water_max_pos, water_tint );
		glm::vec2 ul = glm::vec2( view_min.x, view_max.y );

		draw.draw( *sprite_faucet, glm::vec2( 0, view_max.y ) );

		draw.draw( *sprite_water_bottle,  bottle_drop ? bottle_pos : mouse_screen_pos, 1.0f, shake );
	}

	{
		DrawSprites draw( *text_sprites, view_min, view_max, drawable_size, DrawSprites::AlignPixelPerfect );
		if( last_filled_percent >= 0.0f )		
		{
			std::string percent_indicator = std::to_string( (int)std::round( last_filled_percent ) ) + "- filled";
			draw.draw_text( percent_indicator, glm::vec2( 150, 200 ) );
		}
	}
	GL_ERRORS(); //did the DrawSprites do something wrong?
}
