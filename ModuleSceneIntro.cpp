#include "Globals.h"
#include "Application.h"
#include "ModuleRender.h"
#include "ModuleSceneIntro.h"
#include "ModuleInput.h"
#include "ModuleTextures.h"
#include "ModuleAudio.h"
#include "ModulePhysics.h"
#include "ModulePlayer.h"

ModuleSceneIntro::ModuleSceneIntro(Application* app, bool start_enabled) : Module(app, start_enabled)
{
	circle = PinballMap = PinballMap_2nd_Layer = Texture = NULL;
	ray_on = false;
	sensed = false;
	srand(time(NULL));

	//initialize Ball dimension sprites
	ball_rect[0] = { 0, 0, 26, 26 };
	ball_rect[1] = { 26, 0, 26, 26 };
	ball_rect[2] = { 54, 0, 26, 26 };
	ball_rect[3] = { 81, 0, 26, 26 };
	ball_rect[4] = { 108, 0, 26, 26 };
	ball_rect[5] = { 136, 0, 26, 26 };
	ball_rect[6] = { 163, 0, 26, 26 };

	//initialize leds
	blue_button_led = { 1, 13, 20, 16 };

	black_box_leds[0] = { 121, 1, 20, 12 };
	black_box_leds[1] = { 99, 1, 20, 11 };
	black_box_leds[2] = { 77, 1, 19, 11 };
	black_box_leds[3] = { 57, 1, 18, 10 };
	black_box_leds[4] = { 36, 1, 19, 8 };

	blue_led = { 3, 1, 13, 7 };

	red_arrow = { 16, 0, 19, 10 };

	red_leds[0] = { 29, 14, 23, 19 };
	red_leds[1] = { 55, 14, 23, 19 };

	save_ball = { 110, 15, 30, 29 };

	Extra_ball = { 83,14,25,22 };

	save_ball = { 110, 15, 30, 29 };

	ball_panel[0] = { 3, 45, 88, 24 };
	ball_panel[1] = { 3, 71, 88, 24 };
	ball_panel[2] = { 3, 95, 88, 24 };
	ball_panel[3] = { 3, 119, 88, 24 };
	ball_panel[4] = { 3, 145, 88, 24 };

	red_panel_rect = { 131, 45, 15, 26 };

	ramp_cover = { 92, 44, 39, 127 };
	Game_over_rect = { 3, 171, 134, 27 };
	Number_of_Ball = 1;
}

ModuleSceneIntro::~ModuleSceneIntro()
{}

// Load assets
bool ModuleSceneIntro::Start()
{
	LOG("Loading Intro assets");
	bool ret = true;

	App->renderer->camera.x = App->renderer->camera.y = 0;

	App->audio->PlayMusic("pinball/Audio/Music/Music.ogg", 0.0f);

	circle = App->textures->Load("pinball/Textures/Ball.png");
	PinballMap = App->textures->Load("pinball/Textures/Map_Pinball.png");
	PinballMap_2nd_Layer = App->textures->Load("pinball/Textures/Map_Pinball_2nd_Layer.png");
	
	//Load Leds Textures
	Texture = App->textures->Load("pinball/Textures/SpriteSheet.png");

	//Loading Sound Fx
	left_triangle = App->audio->LoadFx("pinball/Audio/Fx/LeftTriangle.wav");
	right_triangle = App->audio->LoadFx("pinball/Audio/Fx/LeftTriangle.wav");
	orange_button = App->audio->LoadFx("pinball/Audio/Fx/OrangeButton.wav");
	led_activation = App->audio->LoadFx("pinball/Audio/Fx/LedActivation.wav");
	diana = App->audio->LoadFx("pinball/Audio/Fx/Diana.wav");
	turbine = App->audio->LoadFx("pinball/Audio/Fx/Turbine.wav");
	red_panel = App->audio->LoadFx("pinball/Audio/Fx/RedPanel.wav");
	led_reactivation = App->audio->LoadFx("pinball/Audio/Fx/LedReactivation.wav");
	all_led_activation = App->audio->LoadFx("pinball/Audio/Fx/3LedActivation.wav");
	blue_button = App->audio->LoadFx("pinball/Audio/Fx/BlueButton.wav");
	start = App->audio->LoadFx("pinball/Audio/Fx/Start.wav");
	saved_ball = App->audio->LoadFx("pinball/Audio/Fx/SavedBall.wav");
	launch = App->audio->LoadFx("pinball/Audio/Fx/Start.wav");
	ball_positioning = App->audio->LoadFx("pinball/Audio/Fx/BallPositioning.wav");


	//Put False all Bools
	for (int i = 0; i < 4; i++)
		Leds_Arrow[i] = false;
	for (int i = 0; i < 3; i++)
		B_UP_LED[i] = false;
	for (int i = 0; i < 4; i++)
		Leds_Reds[i] = false;
	for (int i = 0; i < 4; i++)
		Arrow_Panel[i] = false;

	//Sensors of Start and Game Over
	Sup_Button = App->physics->CreateCircle(142, 232, 2, false, FLOOR_1, FLOOR_1);
	Lose_sensor = App->physics->CreateRectangleSensor(SCREEN_WIDTH / 2, SCREEN_HEIGHT + 10, SCREEN_WIDTH / 2, 10, GAME_OVER);

	CreateMap();
	CreateElements();
	CreateSensors();

	return ret;
}

// Load assets
bool ModuleSceneIntro::CleanUp()
{
	LOG("Unloading Intro scene");
	App->textures->Unload(circle);
	App->textures->Unload(PinballMap);
	App->textures->Unload(PinballMap_2nd_Layer);
	App->textures->Unload(Texture);
	App->textures->Unload(circle);

	return true;
}

// Update: draw background
update_status ModuleSceneIntro::Update()
{
	actualtime = GetTickCount();

	if (App->input->GetKey(SDL_SCANCODE_1) == KEY_DOWN && ModeGOD)
	{
		circles.add(App->physics->CreateCircle(App->input->GetMouseX(), App->input->GetMouseY(), 7, true, BALL_1, BALL_1 | BALL_2 | FLOOR_1));
	}
	if (App->input->GetKey(SDL_SCANCODE_RETURN) == KEY_UP)
	{
		p2List_item<PhysBody*>* tmp = circles.getFirst();

		while (tmp != NULL)
		{
			b2Transform transform = sensors.getLast()->data->body->GetTransform();
			b2Fixture* f = sensors.getLast()->data->body->GetFixtureList();
			b2Shape* shape = f->GetShape();
			if (shape->TestPoint(transform, tmp->data->body->GetPosition()))
			{
				if (tmp->data == Ball_in_start && Save_Ball == false)
				{
					LaunchBall(tmp->data);
					Save_Ball = true;
					Ball_in_start = nullptr;
					CanApplyForce = false;
					Lives_save_now = actualtime;
				}
			}
			tmp = tmp->next;
		}
	}

	// Prepare for raycast ------------------------------------------------------

	iPoint mouse;
	mouse.x = App->input->GetMouseX();
	mouse.y = App->input->GetMouseY();
	int ray_hit = ray.DistanceTo(mouse);

	fVector normal(0.0f, 0.0f);

	//Time of save ball
	if (Save_Ball)
	{
		if (actualtime >= Lives_save_now + 15000)
		{
			last_seconds = true;
		}

		if (actualtime >= Lives_save_now + 20000 && Save_Ball)
		{
			Save_Ball = false;
			last_seconds = false;
		}
	}


	//Check activated doors
	Activate_Deactivate_Doors();


	// All draw functions ------------------------------------------------------
	p2List_item<PhysBody*>* c;


	App->renderer->Blit(PinballMap, 0, 0, NULL);


	//Draw All Leds:
	if (actualtime >= now + 500)
	{
		Leds_intermittent = true;
		if (actualtime >= now + 1000)
		{
			now = actualtime;
			Leds_intermittent = false;
		}
	}
	if (tree_on_raw)
	{
		if (actualtime >= now_3_row + 2000)
		{
			tree_on_raw = false;
			cLed_activated = false;
			lLed_activated = false;
			rLed_activated = false;
		}
	}
	if (arrows_actived)
	{
		if (actualtime >= now_4_arrows + 2000)
		{
			arrows_actived = false;
			for (int i = 0; i < 4; i++)
				Arrow_Panel[i] = false;
		}
	}
	if (all_reds)
	{
		if (actualtime >= now_All_Reds + 2000)
		{
			all_reds = false;
		}
	}

	if (Restart)
	{
		if (App->input->GetKey(SDL_SCANCODE_SPACE) == KEY_UP)
		{
			Restart = false;
			App->player->Restart_game();
		}
	}
	//Drawing Functions
	DrawLeds();
	DrawPanels();


	//Drawing balls of 1st floor
	c = circles.getFirst();
	while (c != NULL)
	{
		if (c->data->body->GetFixtureList()->GetFilterData().categoryBits == BALL_1)
		{
			DrawBall(c->data);
		}
		c = c->next;
	}

	App->renderer->Blit(PinballMap_2nd_Layer, 0, 0, NULL);


	// ray -----------------
	if (ray_on == true)
	{
		fVector destination(mouse.x - ray.x, mouse.y - ray.y);
		destination.Normalize();
		destination *= ray_hit;

		App->renderer->DrawLine(ray.x, ray.y, ray.x + destination.x, ray.y + destination.y, 255, 255, 255);

		if (normal.x != 0.0f)
			App->renderer->DrawLine(ray.x + destination.x, ray.y + destination.y, ray.x + destination.x + normal.x * 25.0f, ray.y + destination.y + normal.y * 25.0f, 100, 255, 100);
	}

	//Saving ball into the Blue Button
	if (App->player->ball_saved)
	{
		Joint_Blue_button = App->physics->CreateJoint(Joint_Blue_button, save, Sup_Button, false, 0, 0, 0, 0, false, 0, 0);
		Blue_Button = actualtime;
		isEnter = true;

		App->player->ball_saved = false;
		time_extra_ball = actualtime;
		can_crate_new_ball = true;
	}

	//Creating new ball if one is introduced into the Blue Button
	if (isEnter)
	{
		if (actualtime >= time_extra_ball + 800 && can_crate_new_ball == true)
		{
			CreateBall();
			Extra_B = circles.getLast()->data;
			Draw_extra = true;
			can_crate_new_ball = false;
			Now_force_new = true;
		}
		if (actualtime >= time_extra_ball + 1250 && Now_force_new)
		{
			Now_force_new = false;
			LaunchBall(circles.getLast()->data);
		}

		if (actualtime >= Blue_Button + 40000 || circles.count() == 2 && actualtime >= Blue_Button + 2000)//40s stay in Button
		{
			App->audio->PlayFx(blue_button);
			App->physics->Destroy_Joint_button();
			save->body->ApplyForceToCenter(b2Vec2(10.0f, 20.0f), true);
			isEnter = false;
			save = nullptr;
		}
	}

	//Drawing the Ball of the ramp 
	c = circles.getFirst();
	while (c != NULL)
	{
		if (c->data->body->GetFixtureList()->GetFilterData().categoryBits == BALL_2)
		{
			DrawBall(c->data);
		}
		c = c->next;
	}

	//Drawing ramp cover
	App->renderer->Blit(Texture, 201, 3, &ramp_cover);

	//Check GAME STATE
	if (Check_Game_Over == true)
	{
		App->physics->Destroy_Joint_Mouse();
		if (Extra_B == lose_ball)
		{
			Draw_extra = false;
			Extra_B = nullptr;
		}
		circles.del(circles.findNode(lose_ball));
		lose_ball->body->GetWorld()->DestroyBody(lose_ball->body);
		lose_ball = nullptr;

		if (App->player->Lives >= 1)
		{
			//If save timer is still active
			if (Save_Ball)
			{
				App->audio->PlayFx(saved_ball);
				ifSave_force = true;
				CanCreate_ball = true;
			}
			else
			{
				if (circles.count() == 1)
				{
					App->player->Lives--;
					if (App->player->Lives == 0)
					{
						App->player->Restart = true;
						Number_of_Ball = 0;
						Restart = true;
					}
					else
					{
						CreateBall();
						Number_of_Ball++;
					}
				}
			}
		}
		Check_Game_Over = false;
	}

	//Create new ball after losing one ball
	if (ifSave_force)
	{
		if (actualtime >= time_new_ball + 1000 && CanCreate_ball)
		{
			CreateBall();
			CanCreate_ball = false;
		}
		if (actualtime >= time_new_ball + 1250)
		{
			LaunchBall(circles.getLast()->data);
			ifSave_force = false;
		}
	}

	return UPDATE_CONTINUE;
}

void ModuleSceneIntro::OnCollision(PhysBody* bodyA, PhysBody* bodyB)
{
	int x, y;

	//CHECKING ALL INTERACTIONS
	if (bodyA != nullptr)
	{
		if (bodyB->type == GAME_OVER)
		{
			App->physics->IsDestroyed = true;
			lose_ball = bodyA;
			Check_Game_Over = true;
			if (Save_Ball)
			{
				time_new_ball = actualtime;
			}
		}

		if (bodyB->type == ORANGE)
		{
			App->audio->PlayFx(orange_button);
			App->player->Score += 10 * App->player->Multiplier;
		}

		if (bodyB->type == R_TRIANGLE)
		{
			App->audio->PlayFx(right_triangle);
		}

		if (bodyB->type == L_TRIANGLE)
		{
			App->audio->PlayFx(left_triangle);
		}

		if (bodyB->type == B_R_LED || bodyB->type == B_L_LED || bodyB->type == B_C_LED)
		{
			App->player->Score += 20 * App->player->Multiplier;
			if (bodyB->type == B_R_LED)
			{
				if (rLed_activated == false)
				{
					if (cLed_activated == true && lLed_activated == true)
					{
						App->audio->PlayFx(all_led_activation);
					}
					else
					{
						App->audio->PlayFx(led_activation);
					}
					rLed_activated = true;
					B_UP_LED[2] = true;
				}
				else
				{
					App->audio->PlayFx(led_reactivation);
				}
			}

			if (bodyB->type == B_L_LED)
			{
				if (lLed_activated == false)
				{
					if (cLed_activated == true && rLed_activated == true)
					{
						App->audio->PlayFx(all_led_activation);
					}
					else
					{
						App->audio->PlayFx(led_activation);
					}

					lLed_activated = true;
					B_UP_LED[0] = true;
				}
				else 
				{
					App->audio->PlayFx(led_reactivation);
				}
			}

			if (bodyB->type == B_C_LED)
			{
				if (cLed_activated == false)
				{
					if (lLed_activated == true && rLed_activated == true)
					{
						App->audio->PlayFx(all_led_activation);
					}
					else
					{
						App->audio->PlayFx(led_activation);
					}
					cLed_activated = true;
					B_UP_LED[1] = true;
				}
				else
				{
					App->audio->PlayFx(led_reactivation);
				}
			}

			if (B_UP_LED[0] && B_UP_LED[1] && B_UP_LED[2])
			{
				App->player->Score += 250 * App->player->Multiplier;
				tree_on_raw = true;
				for (int i = 0; i < 3; i++)
					B_UP_LED[i] = false;
				now_3_row = actualtime;
			}
		}

		if (bodyB->type == RED_LED_1 || bodyB->type == RED_LED_2 || bodyB->type == RED_LED_3 || bodyB->type == RED_LED_4)
		{
			if (bodyB->type == RED_LED_1 && all_reds == false)
			{
				if (Leds_Reds[0] == false)
				{
					if (Leds_Reds[1] == true && Leds_Reds[2] == true && Leds_Reds[3] == true)
					{
						App->audio->PlayFx(all_led_activation);
					}
					else
					{
						App->audio->PlayFx(led_activation);
					}
					App->player->Score += 50 * App->player->Multiplier;
					Leds_Reds[0] = true;
				}
				else
				{
					App->audio->PlayFx(led_reactivation);
				}
			}
			if (bodyB->type == RED_LED_2 && all_reds == false)
			{
				if (Leds_Reds[1] == false)
				{
					if (Leds_Reds[0] == true && Leds_Reds[2] == true && Leds_Reds[3] == true)
					{
						App->audio->PlayFx(all_led_activation);
					}
					else
					{
						App->audio->PlayFx(led_activation);
					}
					App->player->Score += 50 * App->player->Multiplier;
					Leds_Reds[1] = true;
				}
				else
				{
					App->audio->PlayFx(led_reactivation);
				}
			}
			if (bodyB->type == RED_LED_3 && all_reds == false)
			{
				if (Leds_Reds[2] == false)
				{
					if (Leds_Reds[1] == true && Leds_Reds[0] == true && Leds_Reds[3] == true)
					{
						App->audio->PlayFx(all_led_activation);
					}
					else
					{
						App->audio->PlayFx(led_activation);
					}
					App->player->Score += 50 * App->player->Multiplier;
					Leds_Reds[2] = true;
				}
				else
				{
					App->audio->PlayFx(led_reactivation);
				}
			}
			if (bodyB->type == RED_LED_4 && all_reds == false)
			{
				if (Leds_Reds[3] == false)
				{
					if (Leds_Reds[1] == true && Leds_Reds[0] == true && Leds_Reds[2] == true)
					{
						App->audio->PlayFx(all_led_activation);
					}
					else
					{
						App->audio->PlayFx(led_activation);
					}
					App->player->Score += 50 * App->player->Multiplier;
					Leds_Reds[3] = true;
				}
				else
				{
					App->audio->PlayFx(led_reactivation);
				}
			}

			if (Leds_Reds[0] && Leds_Reds[1] && Leds_Reds[2] && Leds_Reds[3])
			{
				App->player->Score += 1250 * App->player->Multiplier;
				all_reds = true;
				now_All_Reds = actualtime;
				for (int i = 0; i < 4; i++)
					Leds_Reds[i] = false;
			}
		}

		if (bodyB->type == DIANA)
		{
			App->audio->PlayFx(diana);
			App->player->Score += 175 * App->player->Multiplier;
		}

		if (bodyB->type == TURBINE)
		{
			App->audio->PlayFx(turbine);
			App->player->Score += 1000 * App->player->Multiplier;
		
		}

		if (bodyB->type == RED_PANEL_1 && Arrow_Panel[0] == false)
		{
			Arrow_Panel[0] = true;
			App->audio->PlayFx(red_panel);
			App->player->Score += 175 * App->player->Multiplier;
			Leds_Arrow[0] = true;
		}

		if (bodyB->type == RED_PANEL_2 && Arrow_Panel[1] == false)
		{
			Arrow_Panel[1] = true;
			App->audio->PlayFx(red_panel);
			App->player->Score += 175 * App->player->Multiplier;
			Leds_Arrow[1] = true;
		}
		if (bodyB->type == RED_PANEL_3 && Arrow_Panel[2] == false)
		{
			Arrow_Panel[2] = true;
			App->audio->PlayFx(red_panel);
			App->player->Score += 175 * App->player->Multiplier;
			Leds_Arrow[2] = true;
		}
		if (bodyB->type == RED_PANEL_4 && Arrow_Panel[3] == false)
		{
			Arrow_Panel[3] = true;
			App->audio->PlayFx(red_panel);
			App->player->Score += 175 * App->player->Multiplier;
			Leds_Arrow[3] = true;
		}
		if (Leds_Arrow[0] && Leds_Arrow[1] && Leds_Arrow[2] && Leds_Arrow[3])
		{
			now_4_arrows = actualtime;
			arrows_actived = true;
			App->player->Score += 500 * App->player->Multiplier;
			for (int i = 0; i < 4; i++)
				Leds_Arrow[i] = false;
		}

		if (bodyB->type == BLUE_BUTTON)
		{
			save = bodyA;
			App->player->ball_saved = true;
			App->player->Score += 1000 * App->player->Multiplier;
			if (Leds_Blue_Button == 3)
			{
				Leds_Blue_Button = 0;
			}
			else
				Leds_Blue_Button += 1;
		}

		if (bodyB->type == BLACK_BOX)
		{
			App->player->Multiplier += 1;
			App->player->Score += 1000 * App->player->Multiplier;
			if (Leds_Turbine == 5)
			{
				Leds_Turbine = 0;
			}
			else
				Leds_Turbine += 1;
		}

		if (bodyB->type == START)
		{
			Ball_in_start = bodyA;
			CanApplyForce = true;
		}
		
		if (bodyB->type == ENTRANCE)
		{
			if (deactivate_entrance == false)
			{
				deactivate_entrance = true;
			}
		}

		if (bodyB->type == NOT_ENTRANCE)
		{
			if (activate_entrance == false)
			{
				activate_entrance = true;
			}
		}

		if (bodyB->type == LEFT_PASS)
		{
			if (deactivate_left == false)
			{
				deactivate_left = true;
			}
		}

		if (bodyB->type == LEFT_NOT_PASS)
		{
			if (activate_left == false)
			{
				activate_left = true;
			}
		}

		if (bodyB->type == RIGHT_PASS)
		{
			if (deactivate_right == false)
			{
				deactivate_right = true;
			}
		}

		if (bodyB->type == RIGHT_NOT_PASS)
		{
			if (activate_right == false)
			{
				activate_right = true;
			}
		}

		if (bodyB->type == BLACK_BOX)
		{
			if (deactivate_blackbox == false)
			{
				deactivate_blackbox = true;
			}
		}

		if (bodyB->type == BOX_EXIT_SENSOR)
		{
			if (activate_blackbox == false)
			{
				activate_blackbox = true;
			}
		}
	}
}

bool ModuleSceneIntro::CreateMap()
{
	int rick_head[100] = {
		216, 649,
		40, 547,
		93, 358,
		130, 358,
		156, 344,
		159, 337,
		132, 253,
		127, 238,
		129, 229,
		136, 223,
		145, 221,
		155, 223,
		160, 229,
		165, 240,
		168, 238,
		144, 171,
		174, 63,
		183, 45,
		199, 32,
		216, 23,
		240, 17,
		259, 15,
		404, 15,
		420, 17,
		433, 20,
		446, 24,
		459, 30,
		469, 36,
		478, 44,
		486, 53,
		490, 62,
		650, 642,
		648, 651,
		638, 657,
		623, 656,
		615, 641,
		588, 559,
		582, 541,
		511, 246,
		504, 246,
		500, 249,
		467, 334,
		468, 341,
		496, 357,
		533, 357,
		578, 550,
		397, 648,
		430, 723,
		180, 723,
		210, 660
	};

	int PartUP_right[34] = {
		455, 241,
		483, 178,
		459, 67,
		445, 48,
		426, 38,
		408, 34,
		378, 34,
		380, 46,
		418, 47,
		431, 51,
		446, 60,
		455, 70,
		459, 81,
		474, 150,
		479, 175,
		479, 181,
		453, 240
	};


	int PartUP_center[18] = {
		343, 47,
		343, 40,
		343, 37,
		346, 34,
		349, 34,
		352, 37,
		352, 48,
		349, 50,
		346, 50
	};

	int PartUP_center_2[18] = {
		312, 47,
		312, 40,
		312, 37,
		315, 34,
		318, 34,
		321, 37,
		321, 48,
		318, 50,
		315, 50
	};

	int PartUp_Left[52] = {
		227, 118,
		225, 116,
		205, 116,
		203, 118,
		201, 122,
		196, 134,
		190, 134,
		189, 128,
		198, 82,
		202, 66,
		205, 56,
		212, 48,
		226, 40,
		242, 35,
		256, 33,
		277, 33,
		287, 33,
		287, 46,
		284, 49,
		256, 56,
		249, 60,
		241, 118,
		239, 122,
		234, 121,
		231, 119,
		229, 120
	};

	int Part_Center_1[98] = {
		367, 211,
		366, 193,
		364, 184,
		360, 174,
		355, 168,
		350, 165,
		342, 161,
		332, 158,
		322, 157,
		311, 157,
		300, 158,
		293, 160,
		285, 163,
		279, 168,
		274, 174,
		269, 181,
		266, 191,
		265, 215,
		264, 228,
		264, 234,
		265, 236,
		267, 237,
		270, 237,
		273, 236,
		275, 233,
		276, 209,
		276, 190,
		278, 182,
		281, 177,
		286, 172,
		293, 169,
		304, 166,
		314, 165,
		325, 166,
		335, 169,
		343, 173,
		348, 177,
		353, 184,
		355, 195,
		355, 227,
		355, 232,
		356, 235,
		358, 237,
		361, 238,
		363, 238,
		365, 237,
		367, 235,
		368, 233,
		367, 213
	};

	int Part_Center_2[26] = {
		310, 234,
		309, 229,
		309, 218,
		309, 209,
		310, 204,
		314, 200,
		318, 200,
		321, 203,
		322, 209,
		322, 230,
		319, 235,
		316, 237,
		313, 237
	};

	int Part_Left[20] = {
		127, 392,
		103, 501,
		106, 506,
		198, 554,
		205, 560,
		194, 565,
		95, 510,
		94, 499,
		120, 390,
		125, 387
	};

	int Part_Right[20] = {
		504, 393,
		525, 499,
		526, 508,
		525, 514,
		423, 563,
		411, 560,
		421, 553,
		518, 505,
		500, 394,
		501, 390
	};

	int Left_Triangle[10] = {
		188, 501,
		157, 389,
		142, 467,
		143, 474,
		147, 480
	};

	int Right_Triangle[10] = {
		466, 390,
		431, 501,
		472, 481,
		476, 475,
		477, 467
	};

	int Part_UP_Right_Second[26] = {
		441, 164,
		438, 163,
		436, 160,
		400, 108,
		397, 72,
		399, 68,
		404, 67,
		420, 67,
		427, 70,
		432, 76,
		434, 87,
		445, 160,
		444, 163
	};

	int Ramp[24] = {
		209, 120,
		209, 128,
		197, 128,
		210, 11,
		246, 11,
		237, 128,
		225, 128,
		225, 120,
		232, 120,
		240, 15,
		214, 15,
		203, 120
	};


	pinball.add(App->physics->CreateChain(0, 0, rick_head, 100, NULL, false, FLOOR_1, BALL_1));
	pinball.add(App->physics->CreateChain(0, 0, PartUP_right, 34, NULL, false, FLOOR_1, BALL_1));
	pinball.add(App->physics->CreateChain(0, 0, PartUP_center, 18, NULL, false, FLOOR_1, BALL_1));
	pinball.add(App->physics->CreateChain(0, 0, PartUP_center_2, 18, NULL, false, FLOOR_1, BALL_1));
	pinball.add(App->physics->CreateChain(0, 0, PartUp_Left, 52, NULL, false, FLOOR_1, BALL_1));
	pinball.add(App->physics->CreateChain(0, 0, Part_Center_1, 98, NULL, false, FLOOR_1, BALL_1));
	pinball.add(App->physics->CreateChain(0, 0, Part_Center_2, 26, NULL, false, FLOOR_1, BALL_1));
	pinball.add(App->physics->CreateChain(0, 0, Part_Left, 20, NULL, false, FLOOR_1, BALL_1));
	pinball.add(App->physics->CreateChain(0, 0, Part_Right, 20, NULL, false, FLOOR_1, BALL_1));
	pinball.add(App->physics->CreateChain(0, 0, Left_Triangle, 10, NULL, false, FLOOR_1, BALL_1));
	pinball.add(App->physics->CreateChain(0, 0, Right_Triangle, 10, NULL, false, FLOOR_1, BALL_1));
	pinball.add(App->physics->CreateChain(0, 0, Part_UP_Right_Second, 26, NULL, false, FLOOR_1, BALL_1));
	pinball.add(App->physics->CreateChain(0, 0, Ramp, 24, NULL, false, FLOOR_2, BALL_2));
	circles.add(App->physics->CreateCircle(210, 103, 8, true, BALL_2, BALL_1 | FLOOR_2));

	//The first ball of the game
	CreateBall();

	
	return true;
}

void ModuleSceneIntro::CreateElements()
{
	int Left_Bouncer[16] = {
		201, 501,
		196, 503,
		190, 501,
		157, 388,
		159, 384,
		167, 383,
		171, 388,
		202, 493
	};

	int Right_Bouncer[16] = {
		422, 503,
		416, 500,
		415, 494,
		452, 387,
		457, 383,
		463, 384,
		465, 389,
		429, 501
	};

	int Orange_Bouncer_1[16] = {
		312, 76,
		317, 79,
		327, 79,
		332, 76,
		332, 72,
		327, 69,
		317, 69,
		312, 71
	};

	int Orange_Bouncer_2[16] = {
		277, 100,
		282, 102,
		293, 102,
		298, 100,
		298, 96,
		293, 93,
		282, 93,
		277, 96
	};

	int Orange_Bouncer_3[16] = {
		347, 100,
		352, 102,
		363, 102,
		368, 100,
		368, 96,
		363, 93,
		352, 93,
		347, 96
	};

	int Left_Up_Triangle[8] = {
		133, 256,
		158, 334,
		159, 328,
		135, 254
	};

	int Right_Up_Triangle[8] = {
		501, 247,
		468, 335,
		465, 332,
		498, 249
	};

	int entrance_door[8] = {
		508, 240,
		535, 222,
		537, 229,
		514, 247
	};

	int left_door[8] = {
		278, 16,
		282, 31,
		288, 31,
		284, 16,
	};

	int right_door[8] = {
		379, 16,
		377, 31,
		383, 31,
		385, 16
	};

	int box_exit[8] = {
		397, 67,
		396, 50,
		403, 50,
		404, 67
	};

	solids.add(App->physics->CreatePolygon(0, 0, Left_Bouncer, 16, 1.5f, false, L_TRIANGLE, false, FLOOR_1, BALL_1));
	solids.add(App->physics->CreatePolygon(0, 0, Right_Bouncer, 16, 1.5f, false, R_TRIANGLE, false, FLOOR_1, BALL_1));
	solids.add(App->physics->CreatePolygon(0, 0, Orange_Bouncer_1, 16, 1.5f, false, ORANGE, false, FLOOR_1, BALL_1));
	solids.add(App->physics->CreatePolygon(0, 0, Orange_Bouncer_2, 16, 1.5f, false, ORANGE, false, FLOOR_1, BALL_1));
	solids.add(App->physics->CreatePolygon(0, 0, Orange_Bouncer_3, 16, 1.5f, false, ORANGE, false, FLOOR_1, BALL_1));
	solids.add(App->physics->CreatePolygon(0, 0, Left_Up_Triangle, 8, 1.0f, false, NO_EFFECT, false, FLOOR_1, BALL_1));
	solids.add(App->physics->CreatePolygon(0, 0, Right_Up_Triangle, 8, 1.0f, false, NO_EFFECT, false, FLOOR_1, BALL_1));
	solids.add(App->physics->CreatePolygon(0, 0, entrance_door, 8, NULL, false, ENTRANCE_DOOR, false, FLOOR_1, BALL_1));
	solids.add(App->physics->CreatePolygon(0, 0, left_door, 8, NULL, false, LEFT_DOOR, false, FLOOR_1, BALL_1));
	solids.add(App->physics->CreatePolygon(0, 0, right_door, 8, NULL, false, RIGHT_DOOR, false, FLOOR_1, BALL_1));
	solids.add(App->physics->CreatePolygon(0, 0, box_exit, 8, NULL, false, BOX_EXIT, false, FLOOR_1, BALL_1));
	solids.add(App->physics->CreateCircle(SCREEN_WIDTH / 2 - 36, SCREEN_HEIGHT - 5, 7, false, FLOOR_1, BALL_1));

}

void ModuleSceneIntro::CreateSensors()
{

	int Left_B_Led[8] = {
		299, 45,
		299, 40,
		303, 40,
		303, 45
	};

	int Center_B_Led[8] = {
		330, 45,
		330, 40,
		334, 40,
		334, 45
	};

	int Right_B_Led[8] = {
		361, 45,
		361, 40,
		365, 40,
		365, 45
	};

	int Right_Diana[8] = {
		488, 356,
		472, 346,
		469, 350,
		485, 360
	};

	int Left_Diana[8] = {
		152, 347,
		135, 356,
		137, 359,
		154, 350
	};

	int Turbine[8] = {
		313, 189,
		319, 189,
		319, 169,
		313, 169
	};

	int Red_Panel_1[8] = {
		424, 157,
		420, 158,
		427, 168,
		431, 167
	};

	int Red_Panel_2[8] = {
		416, 144,
		412, 146,
		418, 155,
		422, 153
	};

	int Red_Panel_3[8] = {
		407, 131,
		404, 133,
		411, 143,
		414, 141
	};

	int Red_Panel_4[8] = {
		399, 119,
		396, 121,
		401, 129,
		405, 127
	};

	int Blue_Button[8] = {
		130, 230,
		140, 233,
		144, 232,
		147, 222
	};

	int Red_Led_1[8] = {
		80, 478,
		76, 497,
		83, 499,
		87, 480,
	};

	int Red_Led_4[8] = {
		532, 480,
		536, 498,
		544, 496,
		540, 478
	};

	int Red_Led_2[8] = {
		129, 450,
		125, 467,
		134, 469,
		137, 455,
	};

	int Red_Led_3[8] = {
		485, 455,
		487, 469,
		495, 466,
		493, 451
	};

	int Left_Pass[8] = {
		263, 18,
		265, 30,
		259, 30,
		257, 18,
	};

	int Right_Pass[8] = {
		402, 18,
		399, 30,
		405, 30,
		408, 18
	};

	int Left_Not_Pass[8] = {
		308, 18,
		310, 30,
		304, 30,
		302, 18,
	};

	int Right_Not_Pass[8] = {
		358, 18,
		355, 30,
		362, 30,
		363, 18
	};

	int Entrance[8] = {
		599, 560,
		601, 567,
		620, 551,
		618, 545,
		
	};

	int Not_Entrance[8] = {	
		511, 234,
		508, 228,
		529, 217,
		528, 211
	};

	int Black_Box[8] = {
		437, 89,
		457, 89,
		455, 80,
		435, 80
	};

	int Ramp_sensor[8] = {
		220, 17,
		232, 17,
		232, 13,
		220, 13
	};

	int Box_Exit_Sensor[8] = {
		377, 67,
		376, 50,
		383, 50,
		384, 67
	};

	int Senser_ApplyForce[8] = {
		597, 600,
		638, 600,
		652, 659,
		612, 663
	};


	sensors.add(App->physics->CreatePolygon(0, 0, Left_B_Led, 8, NULL, false, B_L_LED, true, FLOOR_1, BALL_1));
	sensors.add(App->physics->CreatePolygon(0, 0, Center_B_Led, 8, NULL, false, B_C_LED, true, FLOOR_1, BALL_1));
	sensors.add(App->physics->CreatePolygon(0, 0, Right_B_Led, 8, NULL, false, B_R_LED, true, FLOOR_1, BALL_1));
	sensors.add(App->physics->CreatePolygon(0, 0, Right_Diana, 8, NULL, false, DIANA, true, FLOOR_1, BALL_1));
	sensors.add(App->physics->CreatePolygon(0, 0, Left_Diana, 8, NULL, false, DIANA, true, FLOOR_1, BALL_1));
	sensors.add(App->physics->CreatePolygon(0, 0, Turbine, 8, NULL, false, TURBINE, true, FLOOR_1, BALL_1));
	sensors.add(App->physics->CreatePolygon(0, 0, Red_Panel_1, 8, NULL, false, RED_PANEL_1, true, FLOOR_1, BALL_1));
	sensors.add(App->physics->CreatePolygon(0, 0, Red_Panel_2, 8, NULL, false, RED_PANEL_2, true, FLOOR_1, BALL_1));
	sensors.add(App->physics->CreatePolygon(0, 0, Red_Panel_3, 8, NULL, false, RED_PANEL_3, true, FLOOR_1, BALL_1));
	sensors.add(App->physics->CreatePolygon(0, 0, Red_Panel_4, 8, NULL, false, RED_PANEL_4, true, FLOOR_1, BALL_1));
	sensors.add(App->physics->CreatePolygon(0, 0, Blue_Button, 8, NULL, false, BLUE_BUTTON, true, FLOOR_1, BALL_1));
	sensors.add(App->physics->CreatePolygon(0, 0, Red_Led_1, 8, NULL, false, RED_LED_1, true, FLOOR_1, BALL_1));
	sensors.add(App->physics->CreatePolygon(0, 0, Red_Led_2, 8, NULL, false, RED_LED_2, true, FLOOR_1, BALL_1));
	sensors.add(App->physics->CreatePolygon(0, 0, Red_Led_3, 8, NULL, false, RED_LED_3, true, FLOOR_1, BALL_1));
	sensors.add(App->physics->CreatePolygon(0, 0, Red_Led_4, 8, NULL, false, RED_LED_4, true, FLOOR_1, BALL_1));
	sensors.add(App->physics->CreatePolygon(0, 0, Left_Pass, 8, NULL, false, LEFT_PASS, true, FLOOR_1, BALL_1));
	sensors.add(App->physics->CreatePolygon(0, 0, Right_Pass, 8, NULL, false, RIGHT_PASS, true, FLOOR_1, BALL_1));
	sensors.add(App->physics->CreatePolygon(0, 0, Left_Not_Pass, 8, NULL, false, LEFT_NOT_PASS, true, FLOOR_1, BALL_1));
	sensors.add(App->physics->CreatePolygon(0, 0, Right_Not_Pass, 8, NULL, false, RIGHT_NOT_PASS, true, FLOOR_1, BALL_1));
	sensors.add(App->physics->CreatePolygon(0, 0, Entrance, 8, NULL, false, ENTRANCE, true, FLOOR_1, BALL_1));
	sensors.add(App->physics->CreatePolygon(0, 0, Not_Entrance, 8, NULL, false, NOT_ENTRANCE, true, FLOOR_1, BALL_1));
	sensors.add(App->physics->CreatePolygon(0, 0, Black_Box, 8, NULL, false, BLACK_BOX, true, FLOOR_1, BALL_1));
	sensors.add(App->physics->CreatePolygon(0, 0, Ramp_sensor, 8, NULL, false, DIANA, true, FLOOR_2, BALL_2));
	sensors.add(App->physics->CreatePolygon(0, 0, Box_Exit_Sensor, 8, NULL, false, BOX_EXIT_SENSOR, true, FLOOR_1, BALL_1));
	sensors.add(App->physics->CreatePolygon(0, 0, Senser_ApplyForce, 8, NULL, false, START, true, FLOOR_1, BALL_1));


}

void ModuleSceneIntro::DrawLeds()
{
	if (Leds_intermittent)
	{
		//Leds_Blue_Button
		if (Leds_Blue_Button == 0)
		{
			App->renderer->Blit(Texture, 165, 314, &blue_button_led);
		}

		if (Leds_Blue_Button == 1)
		{
			App->renderer->Blit(Texture, 165, 314, &blue_button_led);
		}

		if (Leds_Blue_Button == 2)
		{
			App->renderer->Blit(Texture, 165, 314, &blue_button_led);
		}

		//Black_Box
		if (Leds_Turbine == 0)
		{
			App->renderer->Blit(Texture, 443, 196, &black_box_leds[0]);
		}
		if (Leds_Turbine == 1)
		{
			App->renderer->Blit(Texture, 452, 176, &black_box_leds[1]);
		}
		if (Leds_Turbine == 2)
		{
			App->renderer->Blit(Texture, 452, 155, &black_box_leds[2]);
		}
		if (Leds_Turbine == 3)
		{
			App->renderer->Blit(Texture, 449, 135, &black_box_leds[3]);
		}
		if (Leds_Turbine == 4)
		{
			App->renderer->Blit(Texture, 445, 117, &black_box_leds[4]);
		}

		if (tree_on_raw)
		{
			App->renderer->Blit(Texture, 295, 28, &blue_led);
			App->renderer->Blit(Texture, 325, 28, &blue_led);
			App->renderer->Blit(Texture, 356, 28, &blue_led);
		}

		if (arrows_actived)
		{
			App->renderer->Blit(Texture, 400, 158, &red_arrow);
			App->renderer->Blit(Texture, 392, 145, &red_arrow);
			App->renderer->Blit(Texture, 384, 132, &red_arrow);
			App->renderer->Blit(Texture, 377, 121, &red_arrow);
		}

		if (all_reds)
		{
			App->renderer->Blit(Texture, 84, 425, &red_leds[0]);
			App->renderer->Blit(Texture, 131, 399, &red_leds[0]);
			App->renderer->Blit(Texture, 471, 399, &red_leds[1]);
			App->renderer->Blit(Texture, 516, 425, &red_leds[1]);
		}

		if (last_seconds)
		{
			App->renderer->Blit(Texture, 292, 549, &save_ball);
		}
	}

	//LEDS_Permanents
	if (Leds_Blue_Button > 0)
	{
		if (Leds_Blue_Button == 1)
		{
			App->renderer->Blit(Texture, 165, 314, &blue_button_led);
		}

		if (Leds_Blue_Button == 2)
		{
			App->renderer->Blit(Texture, 165, 314, &blue_button_led);
			App->renderer->Blit(Texture, 156, 285, &blue_button_led);
		}
		if (Leds_Blue_Button == 3)
		{
			App->renderer->Blit(Texture, 165, 314, &blue_button_led);
			App->renderer->Blit(Texture, 156, 285, &blue_button_led);
			App->renderer->Blit(Texture, 148, 259, &blue_button_led);
		}
	}

	if (Leds_Arrow[0] || Leds_Arrow[1] || Leds_Arrow[2] || Leds_Arrow[3])
	{
		if (Leds_Arrow[0])
		{
			App->renderer->Blit(Texture, 400, 158, &red_arrow);
		}
		if (Leds_Arrow[1])
		{
			App->renderer->Blit(Texture, 392, 145, &red_arrow);
		}
		if (Leds_Arrow[2])
		{
			App->renderer->Blit(Texture, 384, 132, &red_arrow);
		}
		if (Leds_Arrow[3])
		{
			App->renderer->Blit(Texture, 377, 121, &red_arrow);
		}
	}

	if (Leds_Turbine > 0)
	{
		if (Leds_Turbine == 1)
		{
			App->renderer->Blit(Texture, 443, 196, &black_box_leds[0]);
		}
		if (Leds_Turbine == 2)
		{
			App->renderer->Blit(Texture, 452, 176, &black_box_leds[1]);
			App->renderer->Blit(Texture, 443, 196, &black_box_leds[0]);
		}
		if (Leds_Turbine == 3)
		{
			App->renderer->Blit(Texture, 452, 176, &black_box_leds[1]);
			App->renderer->Blit(Texture, 443, 196, &black_box_leds[0]);
			App->renderer->Blit(Texture, 452, 155, &black_box_leds[2]);
		}
		if (Leds_Turbine == 4)
		{
			App->renderer->Blit(Texture, 452, 176, &black_box_leds[1]);
			App->renderer->Blit(Texture, 443, 196, &black_box_leds[0]);
			App->renderer->Blit(Texture, 452, 155, &black_box_leds[2]);
			App->renderer->Blit(Texture, 449, 135, &black_box_leds[3]);
		}
		if (Leds_Turbine == 5)
		{
			App->renderer->Blit(Texture, 452, 176, &black_box_leds[1]);
			App->renderer->Blit(Texture, 443, 196, &black_box_leds[0]);
			App->renderer->Blit(Texture, 452, 155, &black_box_leds[2]);
			App->renderer->Blit(Texture, 449, 135, &black_box_leds[3]);
			App->renderer->Blit(Texture, 445, 117, &black_box_leds[4]);
		}
	}

	if (B_UP_LED[0] || B_UP_LED[1] || B_UP_LED[2])
	{
		if (B_UP_LED[0])
		{
			App->renderer->Blit(Texture, 295, 28, &blue_led);
		}
		if (B_UP_LED[1])
		{
			App->renderer->Blit(Texture, 325, 28, &blue_led);
		}
		if (B_UP_LED[2])
		{
			App->renderer->Blit(Texture, 356, 28, &blue_led);
		}
	}

	if (Leds_Reds[0] || Leds_Reds[1] || Leds_Reds[2] || Leds_Reds[3])
	{
		if (Leds_Reds[0])//Left
		{
			App->renderer->Blit(Texture, 84, 425, &red_leds[0]);
		}
		if (Leds_Reds[1])//Left
		{
			App->renderer->Blit(Texture, 131, 399, &red_leds[0]);
		}
		if (Leds_Reds[2])//right
		{
			App->renderer->Blit(Texture, 471, 399, &red_leds[1]);
		}
		if (Leds_Reds[3])//right
		{
			App->renderer->Blit(Texture, 516, 425, &red_leds[1]);
		}
	}

	if (Save_Ball && last_seconds == false)
	{
		App->renderer->Blit(Texture, 292, 549, &save_ball);
	}
	if (Draw_extra)
	{
		App->renderer->Blit(Texture, 298, 500, &Extra_ball);
	}
}

void ModuleSceneIntro::DrawPanels()
{
	if (Restart)
	{
		App->renderer->Blit(Texture, 250, 327, &Game_over_rect);
	}

	if (Number_of_Ball == 1)
	{
		App->renderer->Blit(Texture, 275, 325, &ball_panel[0]);
	}
	else if (Number_of_Ball == 2)
	{
		App->renderer->Blit(Texture, 275, 327, &ball_panel[1]);
	}
	else if (Number_of_Ball == 3)
	{
		App->renderer->Blit(Texture, 275, 327, &ball_panel[2]);
	}
	else if (Number_of_Ball == 4)
	{
		App->renderer->Blit(Texture, 275, 327, &ball_panel[3]);
	}
	else if (Number_of_Ball == 5)
	{
		App->renderer->Blit(Texture, 275, 327, &ball_panel[4]);
	}

	if ((Leds_Arrow[3]) == false && arrows_actived == false)
	{
		App->renderer->Blit(Texture, 393, 105, &red_panel_rect);
	}
	if ((Leds_Arrow[2]) == false && arrows_actived == false)
	{
		App->renderer->Blit(Texture, 401, 117, &red_panel_rect);
	}
	if ((Leds_Arrow[1]) == false && arrows_actived == false)
	{
		App->renderer->Blit(Texture, 409, 129, &red_panel_rect);
	}
	if ((Leds_Arrow[0]) == false && arrows_actived == false)
	{
		App->renderer->Blit(Texture, 418, 142, &red_panel_rect);
	}
	

}


void ModuleSceneIntro::CreateBall()
{
	int h = rand() % 5;
	int w = rand() % 5;
	App->audio->PlayFx(ball_positioning);
	circles.add(App->physics->CreateCircle(623 + w, 640 + h, 8, true, BALL_1, BALL_1 | BALL_2 | FLOOR_1));
}

void ModuleSceneIntro::LaunchBall(PhysBody* ball)
{
	App->audio->PlayFx(launch);
	ball->body->ApplyForceToCenter(b2Vec2(0.0f, -120.0f), true);
}


//Unidirectional doors mechanism
void ModuleSceneIntro::Activate_Deactivate_Doors()
{
	//ENTRANCE DOOR
	if (deactivate_entrance == true)
	{
		p2List_item<PhysBody*>* c;
		c = solids.getFirst();
		while (c != NULL)
		{
			if (c->data->type == ENTRANCE_DOOR)
			{
				c->data->body->SetActive(false);
				deactivate_entrance = false;
				break;
			}
			c = c->next;
		}
	}

	if (activate_entrance == true)
	{
		p2List_item<PhysBody*>* c;
		c = solids.getFirst();
		while (c != NULL)
		{
			if (c->data->type == ENTRANCE_DOOR)
			{
				c->data->body->SetActive(true);
				activate_entrance = false;
				break;
			}
			c = c->next;
		}
	}

	//LEFT DOOR
	if (deactivate_left == true)
	{
		p2List_item<PhysBody*>* c;
		c = solids.getFirst();
		while (c != NULL)
		{
			if (c->data->type == LEFT_DOOR)
			{
				c->data->body->SetActive(false);
				deactivate_left = false;
				break;
			}
			c = c->next;
		}
	}

	if (activate_left == true)
	{
		p2List_item<PhysBody*>* c;
		c = solids.getFirst();
		while (c != NULL)
		{
			if (c->data->type == LEFT_DOOR)
			{
				c->data->body->SetActive(true);
				activate_left = false;
				break;
			}
			c = c->next;
		}
	}

	//RIGHT DOOR
	if (deactivate_right == true)
	{
		p2List_item<PhysBody*>* c;
		c = solids.getFirst();
		while (c != NULL)
		{
			if (c->data->type == RIGHT_DOOR)
			{
				c->data->body->SetActive(false);
				deactivate_right = false;
				break;
			}
			c = c->next;
		}
	}

	if (activate_right == true)
	{
		p2List_item<PhysBody*>* c;
		c = solids.getFirst();
		while (c != NULL)
		{
			if (c->data->type == RIGHT_DOOR)
			{
				c->data->body->SetActive(true);
				activate_right = false;
				break;
			}
			c = c->next;
		}
	}

	//BLACK_BOX
	if (deactivate_blackbox == true)
	{
		p2List_item<PhysBody*>* c;
		c = solids.getFirst();
		while (c != NULL)
		{
			if (c->data->type == BOX_EXIT)
			{
				c->data->body->SetActive(false);
				deactivate_blackbox = false;
				break;
			}
			c = c->next;
		}
	}

	if (activate_blackbox == true)
	{
		p2List_item<PhysBody*>* c;
		c = solids.getFirst();
		while (c != NULL)
		{
			if (c->data->type == BOX_EXIT)
			{
				c->data->body->SetActive(true);
				activate_blackbox = false;
				break;
			}
			c = c->next;
		}
	}

}

void ModuleSceneIntro::DrawBall(const PhysBody * ball)
{
	if (ball != nullptr)
	{
		int x, y;
		ball->GetPosition(x, y);

		if (y > 0 && y <= SCREEN_HEIGHT / 7)
		{
			App->renderer->Blit(circle, x - 4.7f, y - 3.0f, &ball_rect[6]);
		}
		else if (y > SCREEN_HEIGHT / 7 && y <= 2 * SCREEN_HEIGHT / 7)
		{
			App->renderer->Blit(circle, x - 4.7f, y - 3.0f, &ball_rect[5]);
		}
		else if (y > 2 * SCREEN_HEIGHT / 7 && y <= 3 * SCREEN_HEIGHT / 7)
		{
			App->renderer->Blit(circle, x - 4.7f, y - 3.0f, &ball_rect[4]);
		}
		else if (y > 3 * SCREEN_HEIGHT / 7 && y <= 4 * SCREEN_HEIGHT / 7)
		{
			App->renderer->Blit(circle, x - 4.7f, y - 3.0f, &ball_rect[3]);
		}
		else if (y > 4 * SCREEN_HEIGHT / 7 && y <= 5 * SCREEN_HEIGHT / 7)
		{
			App->renderer->Blit(circle, x - 4.7f, y - 3.0f, &ball_rect[2]);
		}
		else if (y > 5 * SCREEN_HEIGHT / 7 && y <= 6 * SCREEN_HEIGHT / 7)
		{
			App->renderer->Blit(circle, x - 4.7f, y - 3.0f, &ball_rect[1]);
		}
		else if (y > 6 * SCREEN_HEIGHT / 7 && y <= SCREEN_HEIGHT)
		{
			App->renderer->Blit(circle, x - 4.7f, y - 3.0f, &ball_rect[0]);
		}
	}
}


