
/*
 * WaterMode implements the water filling game
 *
 */

#include "Mode.hpp"
#include "Sound.hpp"

struct WaterMode : Mode {
	WaterMode();
	virtual ~WaterMode();

	virtual bool handle_event(SDL_Event const &evt, glm::uvec2 const &window_size) override;
	virtual void update(float elapsed) override;
	virtual void draw(glm::uvec2 const &drawable_size) override;
	
	glm::vec2 view_min = glm::vec2(0,0);
	glm::vec2 view_max = glm::vec2(256, 224);

	glm::vec2 mouse_screen_pos = glm::vec2( 0, 0 );
	glm::vec2 water_spawn_pos = glm::vec2( 124, 190 );
	float water_radius = 8;
	bool is_water_entering_bottle = false;
	float bottle_fill_time = 0.0f;
	float bottle_fill_max = 5.0f;

	bool bottle_drop = false;
	float handing_off_border_size = 10;
	glm::vec2 bottle_pos = glm::vec2( 0, 0 );
	glm::vec2 bottle_velocity = glm::vec2( 0, 0 );
	glm::vec2 gravity = glm::vec2(0,-900.0f);
	float respawn_height = -200;
	float last_filled_percent = -1.0f;

	//------ background music -------
	std::shared_ptr< Sound::PlayingSample > filling_sound;
	std::shared_ptr< Sound::PlayingSample > faucet_sound;
};
