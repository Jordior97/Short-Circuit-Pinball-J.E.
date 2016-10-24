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
	circle = box = PinballMap = PinballMap_2nd_Layer = NULL;
	ray_on = false;
	sensed = false;
}

ModuleSceneIntro::~ModuleSceneIntro()
{}

// Load assets
bool ModuleSceneIntro::Start()
{
	LOG("Loading Intro assets");
	bool ret = true;

	App->renderer->camera.x = App->renderer->camera.y = 0;

	circle = App->textures->Load("pinball/Ball.png");
	box = App->textures->Load("pinball/crate.png");
	PinballMap = App->textures->Load("pinball/Map_Pinball.png");
	PinballMap_2nd_Layer = App->textures->Load("pinball/Map_Pinball_2nd_Layer.png");

	//Loading Sound Fx
	left_triangle = App->audio->LoadFx("pinball/Audio/Fx/LeftTriangle.wav");
	right_triangle = App->audio->LoadFx("pinball/Audio/Fx/LeftTriangle.wav");
	orange_button = App->audio->LoadFx("pinball/Audio/Fx/OrangeButton.wav");
	led_activation = App->audio->LoadFx("pinball/Audio/Fx/LedActivation.wav");
	diana = App->audio->LoadFx("pinball/Audio/Fx/Diana.wav");
	turbine = App->audio->LoadFx("pinball/Audio/Fx/Turbine.wav");
	red_panel = App->audio->LoadFx("pinball/Audio/Fx/RedPanel.wav");
	blue_led_reactivation = App->audio->LoadFx("pinball/Audio/Fx/LedReactivation.wav");
	all_led_activation = App->audio->LoadFx("pinball/Audio/Fx/LedReactivation.wav");

	Lose_sensor = App->physics->CreateRectangleSensor(SCREEN_WIDTH / 2, SCREEN_HEIGHT + 10, SCREEN_WIDTH / 2, 10, GAME_OVER);

	CreateMap();
	CreateBouncers();
	CreateSensors();

	return ret;
}

// Load assets
bool ModuleSceneIntro::CleanUp()
{
	LOG("Unloading Intro scene");

	return true;
}

// Update: draw background
update_status ModuleSceneIntro::Update()
{

	if (App->input->GetKey(SDL_SCANCODE_1) == KEY_DOWN)
	{
		circles.add(App->physics->CreateCircle(App->input->GetMouseX(), App->input->GetMouseY(), 7, true));
	}

	/*if(App->input->GetKey(SDL_SCANCODE_2) == KEY_DOWN)
	{
	boxes.add(App->physics->CreateRectangle(App->input->GetMouseX(), App->input->GetMouseY(), 100, 50));
	}*/

	if (App->input->GetKey(SDL_SCANCODE_SPACE) == KEY_UP)
	{
		if (circles.getFirst() != NULL)
		{
			circles.getFirst()->data->body->ApplyForceToCenter(b2Vec2(0.0f, -120.0f), true);
		}
	}

	if (App->input->GetKey(SDL_SCANCODE_3) == KEY_DOWN)
	{
		// Pivot 0, 0

	}

	// Prepare for raycast ------------------------------------------------------

	iPoint mouse;
	mouse.x = App->input->GetMouseX();
	mouse.y = App->input->GetMouseY();
	int ray_hit = ray.DistanceTo(mouse);

	fVector normal(0.0f, 0.0f);

	// All draw functions ------------------------------------------------------
	p2List_item<PhysBody*>* c;


	App->renderer->Blit(PinballMap, 0, 0, NULL);

	c = circles.getFirst();

	while (c != NULL)
	{
		int x, y;
		c->data->GetPosition(x, y);
		App->renderer->Blit(circle, x - 4.7f, y - 3.0f, NULL, 1.0f);
		c = c->next;
	}

	c = boxes.getFirst();

	while (c != NULL)
	{
		int x, y;
		c->data->GetPosition(x, y);
		App->renderer->Blit(box, x - 10, y - 5, NULL, 1.0f, c->data->GetRotation());
		if (ray_on)
		{
			int hit = c->data->RayCast(ray.x, ray.y, mouse.x, mouse.y, normal.x, normal.y);
			if (hit >= 0)
				ray_hit = hit;
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

	if (App->player->ball_saved == true)
	{
		save->body->GetWorld()->DestroyBody(save->body);
		circles.add(App->physics->CreateCircle(620, 600, 8, true));
		save = nullptr;
		App->player->ball_saved = false;
	}


	//Check GAME STATE
	if (Game_Over == true)
	{
		lose_ball->body->GetWorld()->DestroyBody(lose_ball->body);
		lose_ball = nullptr;
		
		if (App->player->Extra_Balls > 0)
		{
			App->player->Extra_Balls--;
			circles.add(App->physics->CreateCircle(150, 250, 8, true));
			circles.getFirst()->data->body->ApplyForceToCenter(b2Vec2(15, 30), true);
		}

		else if (App->player->Lives > 1)
		{
			circles.add(App->physics->CreateCircle(620, 600, 8, true));
			App->player->Lives--;
		}
		else
		{
			App->player->Restart_game();

		}

		Game_Over = false;
	}

	return UPDATE_CONTINUE;
}

void ModuleSceneIntro::OnCollision(PhysBody* bodyA, PhysBody* bodyB)
{
	int x, y;

	//App->audio->PlayFx(bonus_fx);

	if (bodyA != nullptr)
	{
		if (bodyB->type == GAME_OVER)
		{
			lose_ball = bodyA;
			Game_Over = true;
		}

		if (bodyB->type == ORANGE)
		{
			App->audio->PlayFx(orange_button);
			App->player->Score += 10;
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
			App->player->Score += 20;
			if (bodyB->type == B_R_LED)
			{
				if (rLed_activated == false)
				{
					App->audio->PlayFx(led_activation);
					rLed_activated = true;
				}
				else
				{
					App->audio->PlayFx(blue_led_reactivation);
				}
			}

			if (bodyB->type == B_L_LED)
			{
				if (lLed_activated == false)
				{
					App->audio->PlayFx(led_activation);
					lLed_activated = true;
				}
				else
				{
					App->audio->PlayFx(blue_led_reactivation);
				}
			}
			if (bodyB->type == B_C_LED)
			{
				if (cLed_activated == false)
				{
					App->audio->PlayFx(led_activation);
					cLed_activated = true;
				}
				else
				{
					App->audio->PlayFx(blue_led_reactivation);
				}
			}
		}

		if (bodyB->type == DIANA)
		{
			App->audio->PlayFx(diana);
			App->player->Score += 175;
		}

		if (bodyB->type == TURBINE)
		{
			App->audio->PlayFx(turbine);
			App->player->Score += 1000;
		}

		if (bodyB->type == RED_PANEL_1 && Red_Panel1 == false)
		{
			Red_Panel1 = true;
			App->audio->PlayFx(red_panel);
			App->player->Score += 175;
		}

		if (bodyB->type == RED_PANEL_2 && Red_Panel2 == false)
		{
			Red_Panel2 = true;
			App->audio->PlayFx(red_panel);
			App->player->Score += 175;
		}
		if (bodyB->type == RED_PANEL_3 && Red_Panel3 == false)
		{
			Red_Panel3 = true;
			App->audio->PlayFx(red_panel);
			App->player->Score += 175;
		}
		if (bodyB->type == RED_PANEL_4 && Red_Panel4 == false)
		{
			Red_Panel4 = true;
			App->audio->PlayFx(red_panel);
			App->player->Score += 175;
		}

		if (bodyB->type == BLUE_BUTTON)
		{
			save = bodyA;
			App->player->Extra_Balls++;
			App->player->ball_saved = true;
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
		128, 245,
		124, 239,
		125, 226,
		132, 217,
		144, 213,
		155, 214,
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

	ricks.add(App->physics->CreateChain(0, 0, rick_head, 100, 0, false));
	ricks.add(App->physics->CreateChain(0, 0, PartUP_right, 34, 0, false));
	ricks.add(App->physics->CreateChain(0, 0, PartUP_center, 18, 0.5f, false));
	ricks.add(App->physics->CreateChain(0, 0, PartUP_center_2, 18, 0.5f, false));
	ricks.add(App->physics->CreateChain(0, 0, PartUp_Left, 52, 0, false));
	ricks.add(App->physics->CreateChain(0, 0, Part_Center_1, 98, 0, false));
	ricks.add(App->physics->CreateChain(0, 0, Part_Center_2, 26, 0, false));
	ricks.add(App->physics->CreateChain(0, 0, Part_Left, 20, 0, false));
	ricks.add(App->physics->CreateChain(0, 0, Part_Right, 20, 0, false));
	ricks.add(App->physics->CreateChain(0, 0, Left_Triangle, 10, 0, false));
	ricks.add(App->physics->CreateChain(0, 0, Right_Triangle, 10, 0, false));
	ricks.add(App->physics->CreateChain(0, 0, Part_UP_Right_Second, 26, 0, false));

	circles.add(App->physics->CreateCircle(620, 600, 8, true));

	return true;
}


void ModuleSceneIntro::CreateBouncers()
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


	bouncers.add(App->physics->CreatePolygon(0, 0, Left_Bouncer, 16, 1.5f, false, L_TRIANGLE, false));
	bouncers.add(App->physics->CreatePolygon(0, 0, Right_Bouncer, 16, 1.5f, false, R_TRIANGLE, false));
	bouncers.add(App->physics->CreatePolygon(0, 0, Orange_Bouncer_1, 16, 1.5f, false, ORANGE, false));
	bouncers.add(App->physics->CreatePolygon(0, 0, Orange_Bouncer_2, 16, 1.5f, false, ORANGE, false));
	bouncers.add(App->physics->CreatePolygon(0, 0, Orange_Bouncer_3, 16, 1.5f, false, ORANGE, false));
	bouncers.add(App->physics->CreatePolygon(0, 0, Left_Up_Triangle, 8, 1.0f, false, NO_EFFECT, false));
	bouncers.add(App->physics->CreatePolygon(0, 0, Right_Up_Triangle, 8, 1.0f, false, NO_EFFECT, false));

}

void ModuleSceneIntro::CreateSensors()
{
	int Left_B_Led[8] = {
		299, 53,
		299, 40,
		303, 40,
		303, 53
	};

	int Center_B_Led[8] = {
		330, 51,
		330, 40,
		334, 40,
		334, 51
	};

	int Right_B_Led[8] = {
		361, 52,
		361, 41,
		365, 41,
		365, 52
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
		125, 230,
		129, 238,
		158, 229,
		155, 220
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
		278, 18,
		280, 30,
		274, 30,
		272, 18,
	};

	int Right_Pass[8] = {
		387, 18,
		384, 30,
		390, 30,
		393, 18
	};

	int Left_Not_Pass[8] = {
		298, 18,
		300, 30,
		294, 30,
		292, 18,
	};

	int Right_Not_Pass[8] = {
		367, 18,
		362, 30,
		370, 30,
		373, 18
	};

	int Entrance[8] = {
		519, 260,
		521, 267,
		540, 251,
		538, 245,
		
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


	sensors.add(App->physics->CreatePolygon(0, 0, Left_B_Led, 8, NULL, false, B_L_LED, true));
	sensors.add(App->physics->CreatePolygon(0, 0, Center_B_Led, 8, NULL, false, B_C_LED, true));
	sensors.add(App->physics->CreatePolygon(0, 0, Right_B_Led, 8, NULL, false, B_R_LED, true));
	sensors.add(App->physics->CreatePolygon(0, 0, Right_Diana, 8, NULL, false, DIANA, true));
	sensors.add(App->physics->CreatePolygon(0, 0, Left_Diana, 8, NULL, false, DIANA, true));
	sensors.add(App->physics->CreatePolygon(0, 0, Turbine, 8, NULL, false, TURBINE, true));
	sensors.add(App->physics->CreatePolygon(0, 0, Red_Panel_1, 8, NULL, false, RED_PANEL_1, true));
	sensors.add(App->physics->CreatePolygon(0, 0, Red_Panel_2, 8, NULL, false, RED_PANEL_2, true));
	sensors.add(App->physics->CreatePolygon(0, 0, Red_Panel_3, 8, NULL, false, RED_PANEL_3, true));
	sensors.add(App->physics->CreatePolygon(0, 0, Red_Panel_4, 8, NULL, false, RED_PANEL_4, true));
	sensors.add(App->physics->CreatePolygon(0, 0, Blue_Button, 8, NULL, false, BLUE_BUTTON, true));
	sensors.add(App->physics->CreatePolygon(0, 0, Red_Led_1, 8, NULL, false, RED_LED_1, true));
	sensors.add(App->physics->CreatePolygon(0, 0, Red_Led_2, 8, NULL, false, RED_LED_3, true));
	sensors.add(App->physics->CreatePolygon(0, 0, Red_Led_3, 8, NULL, false, RED_LED_4, true));
	sensors.add(App->physics->CreatePolygon(0, 0, Red_Led_4, 8, NULL, false, RED_LED_2, true));
	sensors.add(App->physics->CreatePolygon(0, 0, Left_Pass, 8, NULL, false, LEFT_PASS, true));
	sensors.add(App->physics->CreatePolygon(0, 0, Right_Pass, 8, NULL, false, RIGHT_PASS, true));
	sensors.add(App->physics->CreatePolygon(0, 0, Left_Not_Pass, 8, NULL, false, LEFT_PASS, true));
	sensors.add(App->physics->CreatePolygon(0, 0, Right_Not_Pass, 8, NULL, false, RIGHT_PASS, true));
	sensors.add(App->physics->CreatePolygon(0, 0, Entrance, 8, NULL, false, ENTRANCE, true));
	sensors.add(App->physics->CreatePolygon(0, 0, Not_Entrance, 8, NULL, false, NOT_ENTRANCE, true));
	sensors.add(App->physics->CreatePolygon(0, 0, Black_Box, 8, NULL, false, BLACK_BOX, true));

}