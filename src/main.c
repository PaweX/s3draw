#include <math.h>
#include <s3map.h>
#include <s3map_types.h>
#include <s3dat_ext.h>
#include <GLFW/glfw3.h>

int width = 40*20, height = 32*20;
//int width = 600, height = 400;
double move_x = 0, move_y = 0;

bool redraw = false;
int draw_mode = 2;
bool light = false;

void onresize(GLFWwindow* wnd, int w, int h) {
	width = w;
	height = h;

	glViewport(0, 0, width, height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, width, 0, height, -INT32_MAX, INT32_MAX);
	glMatrixMode(GL_MODELVIEW);
	redraw = true;
}

void onrefresh(GLFWwindow* window) {
	redraw = true;
}

void onkey(GLFWwindow* window, int key, int scancode, int action, int mods) {
	if(action == GLFW_PRESS) {
		if(key == GLFW_KEY_T) {
			draw_mode = draw_mode+1;
			draw_mode %= 3;

			if(draw_mode == 0) {
				move_x = ((move_x+width/2.0)/2.0)-(width/2.0);
				move_y /= 9/8.0;
			} else if(draw_mode == 1) {
				move_x = ((move_x+width/2.0)*2.0)-(width/2.0);
				move_y *= 9/8.0;
			}
		}
		redraw = true;
		if(key == GLFW_KEY_L) {
			light = !light;

			if(light) {
				glEnable(GL_LIGHTING);
				glShadeModel(GL_SMOOTH);
				glEnable(GL_LIGHT0);

				GLfloat mat_specular[] = { 1.0, 1.0, 1.0, 1.0 };
				GLfloat mat_shininess[] = { 50.0 };
	 			GLfloat light_position[] = { 1.0, 1.0, 2.5, 0.0 };
				glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
				glMaterialfv(GL_FRONT, GL_SHININESS, mat_shininess);
				glLightfv(GL_LIGHT0, GL_POSITION, light_position);

			} else {
				glDisable(GL_LIGHTING);
				glDisable(GL_LIGHT0);
			}
		}
	}
}

typedef struct {
	s3dat_t* parent;
	GLuint tex_id;
	uint16_t width;
	uint16_t height;
	int16_t xoff;
	int16_t yoff;
} gltex_t;

#define GLTEX(res) ((gltex_t*)res->data.raw)

void delete_texture(gltex_t* tex) {
	glDeleteTextures(1, &tex->tex_id);
	s3util_free_func(s3dat_memset(tex->parent), tex);
}

s3dat_restype_t gl_bitmap_type = {"gltex", (void (*) (void*)) delete_texture, NULL};

void bitmap_to_gl_handler(s3dat_extracthandler_t* me, s3dat_res_t* res, s3util_exception_t** throws) {
	s3dat_t* handle = me->parent;
	S3DAT_EXHANDLER_CALL(me, res, throws, __FILE__, __func__, __LINE__);

	if(res->type != s3dat_landscape && res->type != s3dat_settler && res->type != s3dat_torso && res->type != s3dat_shadow) return;

	S3DAT_CHECK_TYPE(handle, res, "s3dat_bitmap_t", throws, __FILE__, __func__, __LINE__);

	gltex_t* texhandle = s3util_alloc_func(s3dat_memset(handle), sizeof(gltex_t), throws);
	S3UTIL_HANDLE_EXCEPTION(s3dat_memset(handle), throws, __FILE__, __func__, __LINE__);

	GLuint tex_id;
	glGenTextures(1, &tex_id);
	glBindTexture(GL_TEXTURE_2D, tex_id);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, s3dat_width(res->res), s3dat_height(res->res), 0, GL_RGBA, GL_UNSIGNED_BYTE, s3dat_bmpdata(res->res));
	glBindTexture(GL_TEXTURE_2D, 0);

	texhandle->parent = handle;
	texhandle->tex_id = tex_id;
	texhandle->width = s3dat_width(res->res);
	texhandle->height = s3dat_height(res->res);
	texhandle->xoff = *s3dat_xoff(res->res);
	texhandle->yoff = *s3dat_yoff(res->res);

	res->res->type->deref(res->res->data.raw);

	res->res->data.raw = texhandle;
	res->res->type = &gl_bitmap_type;
}

struct terrain_color{
	s3map_terrain terrain;
	double red;
	double green;
	double blue;
};

struct terrain_color terrain_colors[] = {
	{s3map_terrain_water8, 0, 0, 0.3},
	{s3map_terrain_water7, 0, 0, 0.4},
	{s3map_terrain_water6, 0, 0, 0.5},
	{s3map_terrain_water5, 0, 0, 0.6},
	{s3map_terrain_water4, 0, 0, 0.7},
	{s3map_terrain_water3, 0, 0, 0.8},
	{s3map_terrain_water2, 0, 0, 0.9},
	{s3map_terrain_water1, 0, 0, 1},

	{s3map_terrain_grass, 0, 0.59, 0},
	{s3map_terrain_shore, 0.75, 0.56, 0.19},

	{s3map_terrain_sand1, 0.8, 0.56, 0.19},
	{s3map_terrain_sand2, 0.9, 0.56, 0.19},
	{s3map_terrain_sand3, 1, 0.56, 0.19},

	{s3map_terrain_rock1, 0.5, 0.5, 0.5},
	{s3map_terrain_rock2, 0.4, 0.4, 0.4},
	{s3map_terrain_rock3, 0.3, 0.3, 0.3},

	{s3map_terrain_swamp1, 0, 0.5, 0},
	{s3map_terrain_swamp2, 0, 0.32, 0},
	{s3map_terrain_swamp3, 0, 0.3, 0},

	{s3map_terrain_mud1, 0.4, 0.24, 0},
	{s3map_terrain_mud2, 0.4, 0.22, 0},
	{s3map_terrain_mud3, 0.3, 0.2, 0},

	{s3map_terrain_snow1, 0.7, 0.7, 0.7},
	{s3map_terrain_snow2, 0.8, 0.8, 0.8},
	{s3map_terrain_snow3, 0.9, 0.9, 0.9},
	{0, 0, 0, 0}
};

struct triple_point {
	s3map_terrain type1;
	s3map_terrain alt_type1;
	s3map_terrain type2;

	s3dat_ref_t** type1_tex1;
	s3dat_ref_t** type1_tex2;
	s3dat_ref_t** type2_tex1;
	s3dat_ref_t** type2_tex2;

};

#define F(x, y) fields[(map->map.arg-y-1)*map->map.arg+x]

s3map_t* map;
s3dat_ref_t* ls00;

s3dat_ref_t* ls04;
s3dat_ref_t* ls05;
s3dat_ref_t* ls06;

s3dat_ref_t* ls07;
s3dat_ref_t* ls08;
s3dat_ref_t* ls09;

s3dat_ref_t* ls10;

s3dat_ref_t* ls18;
s3dat_ref_t* ls19;
s3dat_ref_t* ls20;

s3dat_ref_t* ls21;
s3dat_ref_t* ls22;
s3dat_ref_t* ls23;

s3dat_ref_t* ls24;
s3dat_ref_t* ls25;
s3dat_ref_t* ls26;

s3dat_ref_t* ls31;


s3dat_ref_t* ls37;
s3dat_ref_t* ls38;
s3dat_ref_t* ls39;
s3dat_ref_t* ls40;


s3dat_ref_t* ls52;
s3dat_ref_t* ls53;
s3dat_ref_t* ls54;
s3dat_ref_t* ls55;

s3dat_ref_t* ls56;
s3dat_ref_t* ls57;
s3dat_ref_t* ls58;
s3dat_ref_t* ls59;

s3dat_ref_t* ls60;
s3dat_ref_t* ls61;
s3dat_ref_t* ls62;
s3dat_ref_t* ls63;

s3dat_ref_t* ls64;
s3dat_ref_t* ls65;
s3dat_ref_t* ls66;
s3dat_ref_t* ls67;


s3dat_ref_t* ls68;
s3dat_ref_t* ls69;
s3dat_ref_t* ls70;
s3dat_ref_t* ls71;

s3dat_ref_t* ls72;
s3dat_ref_t* ls73;
s3dat_ref_t* ls74;
s3dat_ref_t* ls75;

s3dat_ref_t* ls76;
s3dat_ref_t* ls77;
s3dat_ref_t* ls78;
s3dat_ref_t* ls79;

s3dat_ref_t* ls80;
s3dat_ref_t* ls81;
s3dat_ref_t* ls82;
s3dat_ref_t* ls83;

s3dat_ref_t* ls112;
s3dat_ref_t* ls113;

s3dat_ref_t* ls114;
s3dat_ref_t* ls115;

s3dat_ref_t* ls116;
s3dat_ref_t* ls117;

s3dat_ref_t* ls118;
s3dat_ref_t* ls119;

s3dat_ref_t* ls120;
s3dat_ref_t* ls121;

s3dat_ref_t* ls122;
s3dat_ref_t* ls123;

s3dat_ref_t* ls124;
s3dat_ref_t* ls125;

s3dat_ref_t* ls126;
s3dat_ref_t* ls127;

s3dat_ref_t* ls128;
s3dat_ref_t* ls129;

s3dat_ref_t* ls130;
s3dat_ref_t* ls131;

s3dat_ref_t* ls132;
s3dat_ref_t* ls133;

s3dat_ref_t* ls134;
s3dat_ref_t* ls135;

s3dat_ref_t* ls136;
s3dat_ref_t* ls137;

s3dat_ref_t* ls138;
s3dat_ref_t* ls139;

s3dat_ref_t* ls140;
s3dat_ref_t* ls141;

s3dat_ref_t* ls142;
s3dat_ref_t* ls143;

s3dat_ref_t* ls144;
s3dat_ref_t* ls145;

s3dat_ref_t* ls146;
s3dat_ref_t* ls147;

s3dat_ref_t* ls148;
s3dat_ref_t* ls149;

s3dat_ref_t* ls150;
s3dat_ref_t* ls151;

s3dat_ref_t* ls156;
s3dat_ref_t* ls157;

s3dat_ref_t* ls158;
s3dat_ref_t* ls159;

s3dat_ref_t* ls160;
s3dat_ref_t* ls161;

s3dat_ref_t* ls162;
s3dat_ref_t* ls163;

s3dat_ref_t* ls164;
s3dat_ref_t* ls165;

s3dat_ref_t* ls166;
s3dat_ref_t* ls167;

s3dat_ref_t* ls201;
s3dat_ref_t* ls202;

s3dat_ref_t* ls203;
s3dat_ref_t* ls204;

s3dat_ref_t* ls205;
s3dat_ref_t* ls206;

s3dat_ref_t* ls207;
s3dat_ref_t* ls208;

s3dat_ref_t* ls209;
s3dat_ref_t* ls210;

s3dat_ref_t* ls211;
s3dat_ref_t* ls212;
s3map_field_t* fields = NULL;



struct triple_point triple_points[] = {
	{s3map_terrain_shore, s3map_terrain_shore, s3map_terrain_river1, &ls68, &ls69, &ls70, &ls71},
	{s3map_terrain_shore, s3map_terrain_shore, s3map_terrain_river2, &ls72, &ls73, &ls74, &ls75},
	{s3map_terrain_shore, s3map_terrain_shore, s3map_terrain_river3, &ls76, &ls77, &ls78, &ls79},
	{s3map_terrain_shore, s3map_terrain_shore, s3map_terrain_river4, &ls80, &ls81, &ls82, &ls83},
	{s3map_terrain_shore, s3map_terrain_shore, s3map_terrain_water1, &ls37, &ls38, &ls39, &ls40},
	{s3map_terrain_shore, s3map_terrain_shore, s3map_terrain_grass, &ls112, &ls113, &ls114, &ls115},


	{s3map_terrain_grass, s3map_terrain_shore, s3map_terrain_river1, &ls52, &ls53, &ls54, &ls55},
	{s3map_terrain_grass, s3map_terrain_shore, s3map_terrain_river2, &ls60, &ls61, &ls58, &ls59},
	{s3map_terrain_grass, s3map_terrain_shore, s3map_terrain_river3, &ls64, &ls65, &ls62, &ls63},
	{s3map_terrain_grass, s3map_terrain_shore, s3map_terrain_river4, &ls68, &ls69, &ls66, &ls67},


	{s3map_terrain_grass, s3map_terrain_grass, s3map_terrain_rock1, &ls116, &ls117, &ls118, &ls119},
	{s3map_terrain_rock1, s3map_terrain_rock1, s3map_terrain_rock2, &ls120, &ls121, &ls122, &ls123},
	{s3map_terrain_rock3, s3map_terrain_rock3, s3map_terrain_rock2, &ls124, &ls125, &ls126, &ls127},

	{s3map_terrain_grass, s3map_terrain_grass, s3map_terrain_sand1, &ls128, &ls129, &ls130, &ls131},
	{s3map_terrain_sand1, s3map_terrain_sand1, s3map_terrain_sand2, &ls132, &ls133, &ls134, &ls135},
	{s3map_terrain_sand3, s3map_terrain_sand3, s3map_terrain_sand2, &ls136, &ls137, &ls138, &ls139},

	{s3map_terrain_grass, s3map_terrain_grass, s3map_terrain_mud1, &ls140, &ls141, &ls142, &ls143},
	{s3map_terrain_mud1, s3map_terrain_mud1, s3map_terrain_mud2, &ls144, &ls145, &ls146, &ls147},
	{s3map_terrain_mud3, s3map_terrain_mud3, s3map_terrain_mud2, &ls148, &ls149, &ls150, &ls151},

	{s3map_terrain_rock3, s3map_terrain_rock3, s3map_terrain_snow1, &ls156, &ls157, &ls158, &ls159},
	{s3map_terrain_snow1, s3map_terrain_snow1, s3map_terrain_snow2, &ls160, &ls161, &ls162, &ls163},
	{s3map_terrain_snow3, s3map_terrain_snow3, s3map_terrain_snow2, &ls164, &ls165, &ls166, &ls167},

	{s3map_terrain_grass, s3map_terrain_grass, s3map_terrain_swamp1, &ls201, &ls202, &ls203, &ls204},
	{s3map_terrain_swamp1, s3map_terrain_swamp1, s3map_terrain_swamp2, &ls205, &ls206, &ls207, &ls208},
	{s3map_terrain_swamp3, s3map_terrain_swamp3, s3map_terrain_swamp2, &ls209, &ls210, &ls211, &ls212},
	{0, 0, 0, NULL, NULL, NULL, NULL}
};



void set_color(uint32_t x, uint32_t y, double light) {

	bool terrain_type_found = false;
	double r = 1;
	double g = 1;
	double b = 1;
	struct terrain_color* current_type = &terrain_colors[0];
	while((current_type->red != 0 || current_type->green != 0 || current_type->blue != 0) && !terrain_type_found) {
		if(F(x, y).type == current_type->terrain) {
				r = current_type->red;
				g = current_type->green;
				b = current_type->blue;
				terrain_type_found = true;
		}
		current_type++;
	}
	glColor3f(r*light, g*light, b*light);
}

void tex_coord(uint32_t s, uint32_t t, double width) {
		glTexCoord2d(s/width, 1-(t/width));
}

s3dat_ref_t* type_to_ref(s3map_terrain type) {
	switch(type) {
		case s3map_terrain_grass:
		return ls00;

		case s3map_terrain_shore:
		return ls31;

		case s3map_terrain_water1:
		case s3map_terrain_water2:
		case s3map_terrain_water3:
		case s3map_terrain_water4:
		case s3map_terrain_water5:
		case s3map_terrain_water6:
		case s3map_terrain_water7:
		case s3map_terrain_water8:
		return ls10;

		case s3map_terrain_sand3:
		return ls18;
		case s3map_terrain_sand2:
		return ls19;
		case s3map_terrain_sand1:
		return ls20;

		case s3map_terrain_rock3:
		return ls21;
		case s3map_terrain_rock2:
		return ls22;
		case s3map_terrain_rock1:
		return ls23;

		case s3map_terrain_swamp3:
		return ls07;
		case s3map_terrain_swamp2:
		return ls08;
		case s3map_terrain_swamp1:
		return ls09;

		case s3map_terrain_mud3:
		return ls04;
		case s3map_terrain_mud2:
		return ls05;
		case s3map_terrain_mud1:
		return ls06;

		case s3map_terrain_snow3:
		return ls24;
		case s3map_terrain_snow2:
		return ls25;
		case s3map_terrain_snow1:
		return ls26;
		default:
		return NULL;
	}

}

/*bool triple_point(uint32_t x1, uint32_t y1, uint32_t x2, uint32_t y2, uint32_t x3, uint32_t y3, s3map_terrain primary, s3map_terrain secondary) {
	int primary_count = 0;
	int secondary_count = 0;
	if(F(x1, y1).type == primary) primary_count++;
	if(F(x2, y2).type == primary) primary_count++;
	if(F(x3, y3).type == primary) primary_count++;

	if(F(x1, y1).type == secondary) secondary_count++;
	if(F(x2, y2).type == secondary) secondary_count++;
	if(F(x3, y3).type == secondary) secondary_count++;

	return primary_count == 2 && secondary_count == 1;
}

bool triple_point2(uint32_t x1, uint32_t y1, uint32_t x2, uint32_t y2, uint32_t x3, uint32_t y3, s3map_terrain primary, s3map_terrain secondary, s3map_terrain tertiary) {
	int primary_count = 0;
	int secondary_count = 0;
	int tertiary_count = 0;
	if(F(x1, y1).type == primary) primary_count++;
	if(F(x2, y2).type == primary) primary_count++;
	if(F(x3, y3).type == primary) primary_count++;

	if(F(x1, y1).type == secondary) secondary_count++;
	if(F(x2, y2).type == secondary) secondary_count++;
	if(F(x3, y3).type == secondary) secondary_count++;

	if(F(x1, y1).type == tertiary) tertiary_count++;
	if(F(x2, y2).type == tertiary) tertiary_count++;
	if(F(x3, y3).type == tertiary) tertiary_count++;

	return primary_count > 0 && secondary_count > 0 && primary_count+secondary_count+tertiary_count == 3;
}*/

int triple_point(uint32_t x1, uint32_t y1, uint32_t x2, uint32_t y2, uint32_t x3, uint32_t y3, s3dat_ref_t** tex_return) {
	struct triple_point* tript = &triple_points[0];

	while(tript->type1_tex1 != NULL) {
		int type1_count = 0;
		int type1alt_count = 0;
		int type2_count = 0;
		if(F(x1, y1).type == tript->type1) type1_count++;
		else if(F(x1, y1).type == tript->alt_type1) type1alt_count++;

		if(F(x2, y2).type == tript->type1) type1_count++;
		else if(F(x2, y2).type == tript->alt_type1) type1alt_count++;

		if(F(x3, y3).type == tript->type1) type1_count++;
		else if(F(x3, y3).type == tript->alt_type1) type1alt_count++;


		if(F(x1, y1).type == tript->type2) type2_count++;
		if(F(x2, y2).type == tript->type2) type2_count++;
		if(F(x3, y3).type == tript->type2) type2_count++;

		int tri32 = 0;

		if(type1alt_count != 2 && type2_count > 0 && type1_count+type1alt_count+type2_count == 3) {
			if(type2_count == 2) {
				if(F(x3, y3).type != tript->type2) tri32 = 1;
				else if(F(x2, y2).type != tript->type2) tri32 = 2;
				else if(F(x1, y1).type != tript->type2) tri32 = 3;
			} else {
				if(F(x3, y3).type == tript->type2) tri32 = 1;
				else if(F(x2, y2).type == tript->type2) tri32 = 2;
				else if(F(x1, y1).type == tript->type2) tri32 = 3;
			}
			bool up = y2>y1;

			bool equal = (y2%4) % 2 == 1;
			bool pcolor = (x2%2)^(((y2+1)%4) == !equal);

			if(!up) tri32 = -tri32;

			s3dat_ref_t* tex1 = (type2_count == 2) ? *tript->type2_tex1 : *tript->type1_tex1;
			s3dat_ref_t* tex2 = (type2_count == 2) ? *tript->type2_tex2 : *tript->type1_tex2;

			*tex_return = pcolor ^ (up&!equal) ? tex1 : tex2;
			return tri32;
		}

		tript++;
	}

	return 0;

	/*if(tritex1 && tritex2) {
			bool equal = 1; //(y2%2);
			bool pcolor = 0; //(x2%2); // 1:yellow  (116), 0: blue (117)
			pcolor ^= equal;
			bool up = y2>y1;

			equal = (y2%4) % 2 == 1;
			pcolor = (x2%2)^(((y2+1)%4) == !equal);


			if(F(x1, y1).type == tripri || F(x1, y1).type == triter) {
				if(F(x2, y2).type == trisec) tri32 = 2;
					else tri32 = 1;
			}

			if(F(x1, y1).type == trisec
			&& (F(x2, y2).type == tripri || F(x2, y2).type == triter) ) {
				tri32 = 3;
			}

			if(!up) tri32 = -tri32;

			tex = pcolor ^ (up&!equal) ? tritex1 : tritex2;
		}*/
} 

void tritex_coord(int triangle, int vertex) {
	int abs = triangle < 0 ? -triangle : triangle;
	bool down = abs != triangle;

	double s,t;

	if(vertex == 0) {
		s = 0;
		t = abs == 2 ? 31 : 16;
	} else if(vertex == 1) {
		s = 7;
		t = abs == 2 ? 18 : 29;
	} else if(vertex == 2) {
		s = 8;
		t = abs == 2 ? 18 : 29;
	} else if(vertex == 3) {
		s = 15;
		t = abs == 2 ? 31 : 16;
	}if(abs == 2) {
		s += 8;
	} else if(abs == 3) {
		s += 16;
	}

	if(down) t = 32-t;

	glTexCoord2d((s/32.0), (t/32.0));
}

void draw_triangle(uint32_t x1, uint32_t y1, uint32_t x2, uint32_t y2, uint32_t x3, uint32_t y3) {
	s3dat_ref_t* tex = NULL;

	int tex_width;

	if(type_to_ref(F(x1, y1).type) == type_to_ref(F(x2, y2).type) && type_to_ref(F(x1, y1).type) == type_to_ref(F(x3, y3).type)) {
		tex = type_to_ref(F(x1, y1).type);

		if(tex) tex_width = GLTEX(tex)->width;
	}

	int tri32 = 0;

	 if(!tex) {
		/*s3dat_ref_t* tritex1 = NULL;
		s3dat_ref_t* tritex2 = NULL;
		s3map_terrain tripri;
		s3map_terrain trisec;
		s3map_terrain triter = 0;

		if(triter == 0) triter = tripri;

		if(tritex1 && tritex2) {
			bool equal = 1; //(y2%2);
			bool pcolor = 0; //(x2%2); // 1:yellow  (116), 0: blue (117)
			pcolor ^= equal;
			bool up = y2>y1;

			equal = (y2%4) % 2 == 1;
			pcolor = (x2%2)^(((y2+1)%4) == !equal);


			if(F(x1, y1).type == tripri || F(x1, y1).type == triter) {
				if(F(x2, y2).type == trisec) tri32 = 2;
					else tri32 = 1;
			}

			if(F(x1, y1).type == trisec
			&& (F(x2, y2).type == tripri || F(x2, y2).type == triter) ) {
				tri32 = 3;
			}

			if(!up) tri32 = -tri32;

			tex = pcolor ^ (up&!equal) ? tritex1 : tritex2;
		}*/

		tri32 = triple_point(x1, y1, x2, y2, x3, y3, &tex);
	}

	if(tex) {
		glColor3f(1, 1, 1);
		glBindTexture(GL_TEXTURE_2D, GLTEX(tex)->tex_id);
	}


	glPushMatrix();

	double ymax = y2 > y1 ? 8 : -8;
	uint32_t xpos = x1*16+y1*8;
	uint32_t ypos = y1*9;
	glTranslatef(xpos, ypos, 0);

	glBegin(GL_QUADS);
	if(tri32) tritex_coord(tri32, 0);
	else if(tex) tex_coord(xpos, ypos, tex_width);
	else set_color(x1, y1, 1);
	glVertex2i(0, 0);


	if(tri32) tritex_coord(tri32, 1);
	else if(tex) tex_coord(xpos+7, ypos+ymax, tex_width);
	else set_color(x2, y2, 1);
	glVertex2i(8, ymax);

	if(tri32) tritex_coord(tri32, 2);
	else if(tex) tex_coord(xpos+8, ypos+ymax, tex_width);
	glVertex2i(9, ymax);


	if(tri32) tritex_coord(tri32, 3);
	else if(tex) tex_coord(xpos+16, ypos, tex_width);
	else set_color(x3, y3, 1);
	glVertex2d(17, 0);
	glEnd();
	glPopMatrix();

	if(tex) glBindTexture(GL_TEXTURE_2D, 0);
}

void pertex_coord(uint32_t s, uint32_t t, double width) {
	tex_coord(s, t*1.25, width);
}

void draw_pertriangle(uint32_t x1, uint32_t y1, uint32_t x2, uint32_t y2, uint32_t x3, uint32_t y3) {

	s3dat_ref_t* tex = NULL;

	int tex_width;

	if(type_to_ref(F(x1, y1).type) == type_to_ref(F(x2, y2).type) && type_to_ref(F(x1, y1).type) == type_to_ref(F(x3, y3).type)) {
		tex = type_to_ref(F(x1, y1).type);

		if(tex) tex_width = GLTEX(tex)->width;
	}

	int tri32 = 0;

	if(!tex) {
		tri32 = triple_point(x1, y1, x2, y2, x3, y3, &tex);
	}

	if(tex) {
		glBindTexture(GL_TEXTURE_2D, GLTEX(tex)->tex_id);
	}
	

	double ymax = y2 > y1 ? 8 : -8;
	uint32_t xpos = x1*16+y1*8;
	uint32_t ypos = y1*8;

	if(!light) {
		int diff = F(x1, y1).height-F(x2, y2).height;
		if(F(x3, y3).height == F(x2, y2).height  && y1>y2) diff = 0;
		if(F(x1, y1).height == F(x2, y2).height && y1>y2) diff = F(x3, y3).height-F(x1, y1).height;
		if(y1<y2) diff = -diff;
		glColor3f(0.75+(diff/28.0), 0.75+(diff/28.0), 0.75+(diff/28.0));
	} else {
		glColor3f(1, 1, 1);
	}

	glPushMatrix();
	glTranslatef(xpos, ypos, 0);
	glBegin(GL_QUADS);

	if(tri32) tritex_coord(tri32, 0);
	else if(tex) pertex_coord(xpos, ypos, tex_width);
	glVertex3i(0, F(x1, y1).height, F(x1, y1).height);

	if(tri32) tritex_coord(tri32, 1);
	else if(tex) pertex_coord(xpos+7, ypos+ymax, tex_width);
	glVertex3i(8, ymax+F(x2, y2).height, F(x2, y2).height);

	if(tri32) tritex_coord(tri32, 2);
	else if(tex) pertex_coord(xpos+8, ypos+ymax, tex_width);
	glVertex3i(9, ymax+F(x2, y2).height, F(x2, y2).height);

	if(tri32) tritex_coord(tri32, 3);
	else if(tex) pertex_coord(xpos+16, ypos, tex_width);
	glVertex3d(17, F(x3, y3).height, F(x3, y3).height);
	glEnd();
	glPopMatrix();

	if(tex) glBindTexture(GL_TEXTURE_2D, 0);
}

s3util_exception_t* ex;
int main(int argc, char** argv) {
	if(argc != 2) {
		printf("%s: [mapname]\n", argv[0]);
		return 1;
	}

	map = s3map_new_malloc();
	s3dat_t* dat00 = s3dat_new_malloc();

	s3map_readfile_name(map, argv[1], &ex);
	if(!s3util_catch_exception(&ex)) {
		s3map_delete(map);
		return 1;
	}

	s3dat_readfile_name(dat00, "GFX/Siedler3_00.f8007e01f.dat", &ex);
	if(!s3util_catch_exception(&ex)) {
		s3dat_delete(dat00);
		s3map_delete(map);
		return 1;
	}

	s3dat_add_landscape_blending(dat00, &ex);
	s3util_catch_exception(&ex);

	s3dat_extracthandler_t* exhandler1 = s3dat_new_exhandler(dat00, &ex);
	s3util_catch_exception(&ex);
	exhandler1->call = bitmap_to_gl_handler;
	s3dat_add_extracthandler(dat00, exhandler1);

	glfwInit();


	GLFWwindow* wnd = glfwCreateWindow(width, height, "s3draw", NULL, NULL);
	glfwMakeContextCurrent(wnd);
	glfwSwapInterval(1);
	glfwSetFramebufferSizeCallback(wnd, onresize);
	glfwSetKeyCallback(wnd, onkey);
	glfwSetWindowRefreshCallback(wnd, onrefresh);
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_ALPHA_TEST);
	glEnable(GL_BLEND);

	glAlphaFunc(GL_GREATER, 0.1);
	glBlendFunc(GL_SRC_ALPHA, GL_ZERO);

	onresize(wnd, width, height);

	ls00 = s3dat_extract_landscape(dat00, 0, &ex);
	s3util_catch_exception(&ex);

	ls04 = s3dat_extract_landscape(dat00, 4, &ex);
	s3util_catch_exception(&ex);
	ls05 = s3dat_extract_landscape(dat00, 5, &ex);
	s3util_catch_exception(&ex);
	ls06 = s3dat_extract_landscape(dat00, 6, &ex);
	s3util_catch_exception(&ex);

	ls07 = s3dat_extract_landscape(dat00, 7, &ex);
	s3util_catch_exception(&ex);
	ls08 = s3dat_extract_landscape(dat00, 8, &ex);
	s3util_catch_exception(&ex);
	ls09 = s3dat_extract_landscape(dat00, 9, &ex);
	s3util_catch_exception(&ex);

	ls31 = s3dat_extract_landscape(dat00, 31, &ex);
	s3util_catch_exception(&ex);


	ls37 = s3dat_extract_landscape(dat00, 37, &ex);
	s3util_catch_exception(&ex);
	ls38 = s3dat_extract_landscape(dat00, 38, &ex);
	s3util_catch_exception(&ex);
	ls39 = s3dat_extract_landscape(dat00, 39, &ex);
	s3util_catch_exception(&ex);
	ls40 = s3dat_extract_landscape(dat00, 40, &ex);
	s3util_catch_exception(&ex);

	ls10 = s3dat_extract_landscape(dat00, 10, &ex);
	s3util_catch_exception(&ex);

	ls18 = s3dat_extract_landscape(dat00, 18, &ex);
	s3util_catch_exception(&ex);
	ls19 = s3dat_extract_landscape(dat00, 19, &ex);
	s3util_catch_exception(&ex);
	ls20 = s3dat_extract_landscape(dat00, 20, &ex);
	s3util_catch_exception(&ex);

	ls21 = s3dat_extract_landscape(dat00, 21, &ex);
	s3util_catch_exception(&ex);
	ls22 = s3dat_extract_landscape(dat00, 22, &ex);
	s3util_catch_exception(&ex);
	ls23 = s3dat_extract_landscape(dat00, 23, &ex);
	s3util_catch_exception(&ex);

	ls24 = s3dat_extract_landscape(dat00, 24, &ex);
	s3util_catch_exception(&ex);
	ls25 = s3dat_extract_landscape(dat00, 25, &ex);
	s3util_catch_exception(&ex);
	ls26 = s3dat_extract_landscape(dat00, 26, &ex);
	s3util_catch_exception(&ex);


	ls52 = s3dat_extract_landscape(dat00, 52, &ex);
	s3util_catch_exception(&ex);
	ls53 = s3dat_extract_landscape(dat00, 53, &ex);
	s3util_catch_exception(&ex);
	ls54 = s3dat_extract_landscape(dat00, 54, &ex);
	s3util_catch_exception(&ex);
	ls55 = s3dat_extract_landscape(dat00, 55, &ex);
	s3util_catch_exception(&ex);

	ls56 = s3dat_extract_landscape(dat00, 56, &ex);
	s3util_catch_exception(&ex);
	ls57 = s3dat_extract_landscape(dat00, 57, &ex);
	s3util_catch_exception(&ex);
	ls58 = s3dat_extract_landscape(dat00, 58, &ex);
	s3util_catch_exception(&ex);
	ls59 = s3dat_extract_landscape(dat00, 59, &ex);
	s3util_catch_exception(&ex);

	ls60 = s3dat_extract_landscape(dat00, 60, &ex);
	s3util_catch_exception(&ex);
	ls61 = s3dat_extract_landscape(dat00, 61, &ex);
	s3util_catch_exception(&ex);
	ls62 = s3dat_extract_landscape(dat00, 62, &ex);
	s3util_catch_exception(&ex);
	ls63 = s3dat_extract_landscape(dat00, 63, &ex);
	s3util_catch_exception(&ex);

	ls64 = s3dat_extract_landscape(dat00, 64, &ex);
	s3util_catch_exception(&ex);
	ls65 = s3dat_extract_landscape(dat00, 65, &ex);
	s3util_catch_exception(&ex);
	ls66 = s3dat_extract_landscape(dat00, 66, &ex);
	s3util_catch_exception(&ex);
	ls67 = s3dat_extract_landscape(dat00, 67, &ex);
	s3util_catch_exception(&ex);


	ls68 = s3dat_extract_landscape(dat00, 68, &ex);
	s3util_catch_exception(&ex);
	ls69 = s3dat_extract_landscape(dat00, 69, &ex);
	s3util_catch_exception(&ex);
	ls70 = s3dat_extract_landscape(dat00, 70, &ex);
	s3util_catch_exception(&ex);
	ls71 = s3dat_extract_landscape(dat00, 71, &ex);
	s3util_catch_exception(&ex);

	ls72 = s3dat_extract_landscape(dat00, 72, &ex);
	s3util_catch_exception(&ex);
	ls73 = s3dat_extract_landscape(dat00, 73, &ex);
	s3util_catch_exception(&ex);
	ls74 = s3dat_extract_landscape(dat00, 74, &ex);
	s3util_catch_exception(&ex);
	ls75 = s3dat_extract_landscape(dat00, 75, &ex);
	s3util_catch_exception(&ex);

	ls76 = s3dat_extract_landscape(dat00, 76, &ex);
	s3util_catch_exception(&ex);
	ls77 = s3dat_extract_landscape(dat00, 77, &ex);
	s3util_catch_exception(&ex);
	ls78 = s3dat_extract_landscape(dat00, 78, &ex);
	s3util_catch_exception(&ex);
	ls79 = s3dat_extract_landscape(dat00, 79, &ex);
	s3util_catch_exception(&ex);

	ls80 = s3dat_extract_landscape(dat00, 80, &ex);
	s3util_catch_exception(&ex);
	ls81 = s3dat_extract_landscape(dat00, 81, &ex);
	s3util_catch_exception(&ex);
	ls82 = s3dat_extract_landscape(dat00, 82, &ex);
	s3util_catch_exception(&ex);
	ls83 = s3dat_extract_landscape(dat00, 83, &ex);
	s3util_catch_exception(&ex);


	ls112 = s3dat_extract_landscape(dat00, 112, &ex);
	s3util_catch_exception(&ex);
	ls113 = s3dat_extract_landscape(dat00, 113, &ex);
	s3util_catch_exception(&ex);

	ls114 = s3dat_extract_landscape(dat00, 114, &ex);
	s3util_catch_exception(&ex);
	ls115 = s3dat_extract_landscape(dat00, 115, &ex);
	s3util_catch_exception(&ex);

	ls116 = s3dat_extract_landscape(dat00, 116, &ex);
	s3util_catch_exception(&ex);
	ls117 = s3dat_extract_landscape(dat00, 117, &ex);
	s3util_catch_exception(&ex);

	ls118 = s3dat_extract_landscape(dat00, 118, &ex);
	s3util_catch_exception(&ex);
	ls119 = s3dat_extract_landscape(dat00, 119, &ex);
	s3util_catch_exception(&ex);

	ls120 = s3dat_extract_landscape(dat00, 120, &ex);
	s3util_catch_exception(&ex);
	ls121 = s3dat_extract_landscape(dat00, 121, &ex);
	s3util_catch_exception(&ex);

	ls122 = s3dat_extract_landscape(dat00, 122, &ex);
	s3util_catch_exception(&ex);
	ls123 = s3dat_extract_landscape(dat00, 123, &ex);
	s3util_catch_exception(&ex);

	ls124 = s3dat_extract_landscape(dat00, 124, &ex);
	s3util_catch_exception(&ex);
	ls125 = s3dat_extract_landscape(dat00, 125, &ex);
	s3util_catch_exception(&ex);

	ls126 = s3dat_extract_landscape(dat00, 126, &ex);
	s3util_catch_exception(&ex);
	ls127 = s3dat_extract_landscape(dat00, 127, &ex);
	s3util_catch_exception(&ex);

	ls128 = s3dat_extract_landscape(dat00, 128, &ex);
	s3util_catch_exception(&ex);
	ls129 = s3dat_extract_landscape(dat00, 129, &ex);
	s3util_catch_exception(&ex);

	ls130 = s3dat_extract_landscape(dat00, 130, &ex);
	s3util_catch_exception(&ex);
	ls131 = s3dat_extract_landscape(dat00, 131, &ex);
	s3util_catch_exception(&ex);

	ls132 = s3dat_extract_landscape(dat00, 132, &ex);
	s3util_catch_exception(&ex);
	ls133 = s3dat_extract_landscape(dat00, 133, &ex);
	s3util_catch_exception(&ex);

	ls134 = s3dat_extract_landscape(dat00, 134, &ex);
	s3util_catch_exception(&ex);
	ls135 = s3dat_extract_landscape(dat00, 135, &ex);
	s3util_catch_exception(&ex);

	ls136 = s3dat_extract_landscape(dat00, 136, &ex);
	s3util_catch_exception(&ex);
	ls137 = s3dat_extract_landscape(dat00, 137, &ex);
	s3util_catch_exception(&ex);

	ls138 = s3dat_extract_landscape(dat00, 138, &ex);
	s3util_catch_exception(&ex);
	ls139 = s3dat_extract_landscape(dat00, 139, &ex);
	s3util_catch_exception(&ex);

	ls140 = s3dat_extract_landscape(dat00, 140, &ex);
	s3util_catch_exception(&ex);
	ls141 = s3dat_extract_landscape(dat00, 141, &ex);
	s3util_catch_exception(&ex);

	ls142 = s3dat_extract_landscape(dat00, 142, &ex);
	s3util_catch_exception(&ex);
	ls143 = s3dat_extract_landscape(dat00, 143, &ex);
	s3util_catch_exception(&ex);

	ls144 = s3dat_extract_landscape(dat00, 144, &ex);
	s3util_catch_exception(&ex);
	ls145 = s3dat_extract_landscape(dat00, 145, &ex);
	s3util_catch_exception(&ex);

	ls146 = s3dat_extract_landscape(dat00, 146, &ex);
	s3util_catch_exception(&ex);
	ls147 = s3dat_extract_landscape(dat00, 147, &ex);
	s3util_catch_exception(&ex);

	ls148 = s3dat_extract_landscape(dat00, 148, &ex);
	s3util_catch_exception(&ex);
	ls149 = s3dat_extract_landscape(dat00, 149, &ex);
	s3util_catch_exception(&ex);

	ls150 = s3dat_extract_landscape(dat00, 150, &ex);
	s3util_catch_exception(&ex);
	ls151 = s3dat_extract_landscape(dat00, 151, &ex);
	s3util_catch_exception(&ex);

	ls156 = s3dat_extract_landscape(dat00, 156, &ex);
	s3util_catch_exception(&ex);
	ls157 = s3dat_extract_landscape(dat00, 157, &ex);
	s3util_catch_exception(&ex);

	ls158 = s3dat_extract_landscape(dat00, 158, &ex);
	s3util_catch_exception(&ex);
	ls159 = s3dat_extract_landscape(dat00, 159, &ex);
	s3util_catch_exception(&ex);

	ls160 = s3dat_extract_landscape(dat00, 160, &ex);
	s3util_catch_exception(&ex);
	ls161 = s3dat_extract_landscape(dat00, 161, &ex);
	s3util_catch_exception(&ex);

	ls162 = s3dat_extract_landscape(dat00, 162, &ex);
	s3util_catch_exception(&ex);
	ls163 = s3dat_extract_landscape(dat00, 163, &ex);
	s3util_catch_exception(&ex);

	ls164 = s3dat_extract_landscape(dat00, 164, &ex);
	s3util_catch_exception(&ex);
	ls165 = s3dat_extract_landscape(dat00, 165, &ex);
	s3util_catch_exception(&ex);

	ls166 = s3dat_extract_landscape(dat00, 166, &ex);
	s3util_catch_exception(&ex);
	ls167 = s3dat_extract_landscape(dat00, 167, &ex);
	s3util_catch_exception(&ex);

	ls201 = s3dat_extract_landscape(dat00, 201, &ex);
	s3util_catch_exception(&ex);
	ls202 = s3dat_extract_landscape(dat00, 202, &ex);
	s3util_catch_exception(&ex);

	ls203 = s3dat_extract_landscape(dat00, 203, &ex);
	s3util_catch_exception(&ex);
	ls204 = s3dat_extract_landscape(dat00, 204, &ex);
	s3util_catch_exception(&ex);

	ls205 = s3dat_extract_landscape(dat00, 205, &ex);
	s3util_catch_exception(&ex);
	ls206 = s3dat_extract_landscape(dat00, 206, &ex);
	s3util_catch_exception(&ex);

	ls207 = s3dat_extract_landscape(dat00, 207, &ex);
	s3util_catch_exception(&ex);
	ls208 = s3dat_extract_landscape(dat00, 208, &ex);
	s3util_catch_exception(&ex);

	ls209 = s3dat_extract_landscape(dat00, 209, &ex);
	s3util_catch_exception(&ex);
	ls210 = s3dat_extract_landscape(dat00, 210, &ex);
	s3util_catch_exception(&ex);

	ls211 = s3dat_extract_landscape(dat00, 211, &ex);
	s3util_catch_exception(&ex);
	ls212 = s3dat_extract_landscape(dat00, 212, &ex);
	s3util_catch_exception(&ex);


	s3map_read_mapfields(map, &fields, &ex);
	s3util_catch_exception(&ex);

	double time = glfwGetTime();
	double time_scale = 1;

	//move_x = map->map.arg*(24.0/2);
	//move_y = map->map.arg*(9.0/2);

	while(!glfwWindowShouldClose(wnd)) {
		double old_time = time;
		time = glfwGetTime();
		time_scale = time-old_time;

		glfwPollEvents();
		if(glfwGetKey(wnd, GLFW_KEY_UP) == GLFW_PRESS) {
			move_y += time_scale*180*height/400.0;
			redraw = true;
		} else if(glfwGetKey(wnd, GLFW_KEY_DOWN) == GLFW_PRESS) {
			move_y -= time_scale*180*height/400.0;
			redraw = true;
		}
		if(glfwGetKey(wnd, GLFW_KEY_RIGHT) == GLFW_PRESS) {
			move_x += time_scale*180*width/600.0;
			redraw = true;
		} else if(glfwGetKey(wnd, GLFW_KEY_LEFT) == GLFW_PRESS) {
			move_x -= time_scale*180*width/600.0;
			redraw = true;
		}
		if(move_x < 0) move_x = 0;
		if(move_y < 0) move_y = 0;
		if(draw_mode == 0) {
			if(move_x > (map->map.arg*12-width)) move_x = map->map.arg*12-width;
			if(move_y > (map->map.arg*8-height)) move_y = map->map.arg*8-height;
		} else if(draw_mode == 1 || draw_mode == 2) {
			if(move_x > (map->map.arg*24-width)) move_x = map->map.arg*24-width;
			if(move_y > ((map->map.arg-1)*9-height)) move_y = (map->map.arg-1)*9-height;
		}/* else if(draw_mode == 2) {
			if(move_x > (map->map.arg*24-width)) move_x = map->map.arg*24-width;
			if(move_y > ((map->map.arg-1)*16-height)) move_y = (map->map.arg-1)*16-height;
		}*/


		if(redraw) {
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			glLoadIdentity();
			glTranslatef(round(-move_x), round(-move_y), 0);

			if(draw_mode == 0) {
				uint32_t min_y = move_y/8;
				uint32_t temp_max_y = height/8+min_y+1;
				uint32_t max_y = temp_max_y;
				if(temp_max_y > map->map.arg) max_y = map->map.arg;

				for(uint32_t y = min_y;y != max_y;y++) {
					for(uint32_t x = 0;x != map->map.arg;x++) {
						set_color(x,y, 1);

						glPushMatrix();
						glTranslatef(x*8+y*4, y*8, 0);
						glBegin(GL_QUADS);
						glVertex2d(0, 0);
						glVertex2d(0, 8);
						glVertex2d(8, 8);
						glVertex2d(8, 0);
						glEnd();
						glPopMatrix();
					}
				}
			} else if(draw_mode == 1) {
				uint32_t min_y = move_y/9;
				if(min_y > 0) min_y -= 1;

				uint32_t temp_max_y = ceil(height/9.0+min_y+2);
				uint32_t max_y = temp_max_y;
				if(temp_max_y > map->map.arg-1) max_y = map->map.arg-1;

				
				for(uint32_t y = min_y;y != max_y;y++) {
					for(uint32_t x = 0;x != map->map.arg-1;x++) {
						draw_triangle(x, y, x, y+1, x+1, y);
						draw_triangle(x, y+1, x+1, y, x+1, y+1);
					}
				}
			} else {
				/*uint32_t min_y = move_y/9;
				if(min_y > 0) min_y -= 1;

				uint32_t temp_max_y = ceil(height/9.0+min_y+2);
				uint32_t max_y = temp_max_y;
				if(temp_max_y > map->map.arg-1) max_y = map->map.arg-1;*/

				for(uint32_t y = 0;y != map->map.arg-1;y++) {
					for(uint32_t x = 0;x != map->map.arg-1;x++) {
						draw_pertriangle(x, y, x, y+1, x+1, y);
						draw_pertriangle(x, y+1, x+1, y, x+1, y+1);
					}
				}
			}

				/*glBindTexture(GL_TEXTURE_2D, GLTEX(ls52)->tex_id);
				glPushMatrix();
				glColor3f(1, 1, 1);
				glTranslatef(0, 8, 0);
				glBegin(GL_QUADS);
				glTexCoord2d(9/33.0, 1-(16/16.0));
				glVertex2d(9, 8);

				glTexCoord2d(16/33.0, 1-(8.5/16.0));
				glVertex2d(16, 0.5);

				glTexCoord2d(17/33.0, 1-(8.5/16.0));
				glVertex2d(17, 0.5);

				glTexCoord2d(24/33.0, 1-(16/16.0));
				glVertex2d(24, 8);
				glEnd();

				glBegin(GL_QUADS);
				glTexCoord2d(16/33.0, 1-(8/16.0));
				glVertex2d(16, 0);

				glTexCoord2d(24/33.0, 1-(16/16.0));
				glVertex2d(24, 8);

				glTexCoord2d(25/33.0, 1-(16/16.0));
				glVertex2d(25, 8);

				glTexCoord2d(33/33.0, 1-(8/16.0));
				glVertex2d(33, 0);
				glEnd();

				glColor3f(1, 1, 1);
				glBegin(GL_QUADS);
				glTexCoord2d(0/33.0, 1-(8/16.0));
				glVertex2d(0, 0);

				glTexCoord2d(8/33.0, 1-(16/16.0));
				glVertex2d(8, 8);

				glTexCoord2d(9/33.0, 1-(16/16.0));
				glVertex2d(9, 8);

				glTexCoord2d(17/33.0, 1-(8/16.0));
				glVertex2d(17, 0);
				glEnd();
				glPopMatrix();

				glBegin(GL_QUADS);
				glTexCoord2d(9/33.0, 1-(0/16.0));
				glVertex2d(9, 0);

				glTexCoord2d(16/33.0, 1-(7.5/16.0));
				glVertex2d(16, 7.5);

				glTexCoord2d(17/33.0, 1-(7.5/16.0));
				glVertex2d(17, 7.5);

				glTexCoord2d(24/33.0, 1-(0/16.0));
				glVertex2d(24, 0);
				glEnd();

				glBegin(GL_QUADS);
				glTexCoord2d(16/33.0, 1-(8/16.0));
				glVertex2d(16, 8);

				glTexCoord2d(24/33.0, 1-(0/16.0));
				glVertex2d(24, 0);

				glTexCoord2d(25/33.0, 1-(0/16.0));
				glVertex2d(25, 0);

				glTexCoord2d(33/33.0, 1-(8/16.0));
				glVertex2d(33, 8);
				glEnd();

				glBindTexture(GL_TEXTURE_2D, 0);
				glBegin(GL_QUADS);
				glTexCoord2d(0/33.0, 1-(8/16.0));
				glVertex2d(0, 8);

				glTexCoord2d(8/33.0, 1-(0/16.0));
				glVertex2d(8, 0);

				glTexCoord2d(9/33.0, 1-(0/16.0));
				glVertex2d(9, 0);

				glTexCoord2d(17/33.0, 1-(8/16.0));
				glVertex2d(17, 8);
				/*glTexCoord2d(0, 1-(0/16.0));
				glVertex2d(0, 0);

				glTexCoord2d(0, 1-(8/16.0));
				glVertex2d(0, 8);

				glTexCoord2d(17/33.0, 1-(8/16.0));
				glVertex2d(17, 8);

				glTexCoord2d(17/33.0, 1-(0/16.0));
				glVertex2d(17, 0);
				glEnd();*/


				/*glBindTexture(GL_TEXTURE_2D, 0);
				glColor3f(1, 0, 0);
				glPushMatrix();
				glTranslatef(9, 0, 0);
				glBegin(GL_QUADS);
				glVertex2d(0, 0);
				glVertex2d(7, 8);
				glVertex2d(8.1, 8);
				glVertex2d(16, 0);
				glEnd();

				glTranslatef(0, 8, 0);
				glBegin(GL_QUADS);
				glVertex2d(0, 8);
				glVertex2d(7, 0);
				glVertex2d(8.1, 0);
				glVertex2d(16, 8);
				glEnd();
				glPopMatrix();

				glBegin(GL_QUADS);
				glTexCoord2d(0/32.0, 1-(14/32.0));
				glVertex2d(0, 8);//16);

				glTexCoord2d(7/32.0, 1-(0/32.0));
				glVertex2d(7, 1);
				glTexCoord2d(8/32.0, 1-(0/32.0));
				glVertex2d(8, 1);

				glTexCoord2d(15/32.0, 1-(14/32.0));
				glVertex2d(15, 8);//16);
				glEnd();
				glPopMatrix();
				glBindTexture(GL_TEXTURE_2D, GLTEX(ls52)->tex_id);

				glPushMatrix();
				glTranslatef(16, 0, 0);
				glBegin(GL_TRIANGLES);
				glTexCoord2d(16/32.0, 1-(16/32.0));
				glVertex2d(0, 8);//16);

				glTexCoord2d(24/32.0, 1-(0/32.0));
				glVertex2d(8, 0);

				glTexCoord2d(32/32.0, 1-(16/32.0));
				glVertex2d(16, 8);//16);
				glEnd();
				glPopMatrix();*

				glPushMatrix();
				glTranslatef(8, 0, 0);
				glColor3f(1, 1, 1);
				glBegin(GL_QUADS);
				glTexCoord2d(8/32.0, 1-(0/32.0));
				glVertex2d(0, 0);

				glTexCoord2d(15/32.0, 1-(14/32.0));
				glVertex2d(7, 7);//16);
				glTexCoord2d(16/32.0, 1-(14/32.0));
				glVertex2d(8, 7);//16);

				glTexCoord2d(23/32.0, 1-(0/32.0));
				glVertex2d(15, 0);
				glEnd();
				glPopMatrix();

				glPushMatrix();
				glTranslatef(8, 8, 0);
				glBegin(GL_QUADS);
				glTexCoord2d(8/32.0, 1-(24/32.0));
				glVertex2d(0, 7);//16);

				glTexCoord2d(15/32.0, 1-(14/32.0));
				glVertex2d(7, 0);
				glTexCoord2d(16/32.0, 1-(14/32.0));
				glVertex2d(8, 0);

				glTexCoord2d(23/32.0, 1-(24/32.0));
				glVertex2d(15, 7);//16);
				glEnd();
				glPopMatrix();


				/*glPushMatrix();
				glTranslatef(0, 8, 0);
				glBegin(GL_TRIANGLES);
				glTexCoord2d(0/32.0, 1-(16/32.0));
				glVertex2d(0, 0);//16);

				glTexCoord2d(8/32.0, 1-(32/32.0));
				glVertex2d(8, 8);

				glTexCoord2d(16/32.0, 1-(16/32.0));
				glVertex2d(16, 0);//16);
				glEnd();
				glPopMatrix();

				glPushMatrix();
				glTranslatef(16, 8, 0);
				glBegin(GL_TRIANGLES);
				glTexCoord2d(16/32.0, 1-(16/32.0));
				glVertex2d(0, 0);//16);

				glTexCoord2d(24/32.0, 1-(32/32.0));
				glVertex2d(8, 8);

				glTexCoord2d(32/32.0, 1-(16/32.0));
				glVertex2d(16, 0);//16);
				glEnd();
				glPopMatrix();*/

				/*glBindTexture(GL_TEXTURE_2D, GLTEX(ls52)->tex_id);
				//glBindTexture(GL_TEXTURE_2D, 0);
				glPushMatrix();
				//glScalef(10, 10, 0);

				glBegin(GL_POLYGON);
				glTexCoord2d(8/32.0, 1-0);
				glVertex2d(8, 0);

				glTexCoord2d(24/32.0, 1-0);
				glVertex2d(24, 0);


				glTexCoord2d(32/32.0, 1-(16/32.0));
				glVertex2d(32, 16);

				glTexCoord2d(32/32.0, 1-(17/32.0));
				glVertex2d(32, 17);


				glTexCoord2d(24/32.0, 1-(32/32.0));
				glVertex2d(24, 32);

				glTexCoord2d(8/32.0, 1-(32/32.0));
				glVertex2d(8, 32);


				glTexCoord2d(0/32.0, 1-(17/32.0));
				glVertex2d(0, 17);

				glTexCoord2d(0/32.0, 1-(16/32.0));
				glVertex2d(0, 16);
				glEnd();
				glPopMatrix();*/
			redraw = false;
		}
		glfwSwapBuffers(wnd);
	}
	glfwDestroyWindow(wnd);

	s3dat_unref(ls00);

	s3dat_unref(ls04);
	s3dat_unref(ls05);
	s3dat_unref(ls06);

	s3dat_unref(ls07);
	s3dat_unref(ls08);
	s3dat_unref(ls09);

	s3dat_unref(ls10);

	s3dat_unref(ls18);
	s3dat_unref(ls19);
	s3dat_unref(ls20);

	s3dat_unref(ls21);
	s3dat_unref(ls22);
	s3dat_unref(ls23);

	s3dat_unref(ls24);
	s3dat_unref(ls25);
	s3dat_unref(ls26);
	s3dat_unref(ls31);


	s3dat_unref(ls37);
	s3dat_unref(ls38);
	s3dat_unref(ls39);
	s3dat_unref(ls40);

	s3dat_unref(ls52);
	s3dat_unref(ls53);
	s3dat_unref(ls54);
	s3dat_unref(ls55);

	s3dat_unref(ls56);
	s3dat_unref(ls57);
	s3dat_unref(ls58);
	s3dat_unref(ls59);

	s3dat_unref(ls60);
	s3dat_unref(ls61);
	s3dat_unref(ls62);
	s3dat_unref(ls63);

	s3dat_unref(ls64);
	s3dat_unref(ls65);
	s3dat_unref(ls66);
	s3dat_unref(ls67);


	s3dat_unref(ls68);
	s3dat_unref(ls69);
	s3dat_unref(ls70);
	s3dat_unref(ls71);

	s3dat_unref(ls72);
	s3dat_unref(ls73);
	s3dat_unref(ls74);
	s3dat_unref(ls75);

	s3dat_unref(ls76);
	s3dat_unref(ls77);
	s3dat_unref(ls78);
	s3dat_unref(ls79);

	s3dat_unref(ls80);
	s3dat_unref(ls81);
	s3dat_unref(ls82);
	s3dat_unref(ls83);


	s3dat_unref(ls112);
	s3dat_unref(ls113);

	s3dat_unref(ls114);
	s3dat_unref(ls115);

	s3dat_unref(ls116);
	s3dat_unref(ls117);

	s3dat_unref(ls118);
	s3dat_unref(ls119);

	s3dat_unref(ls120);
	s3dat_unref(ls121);

	s3dat_unref(ls122);
	s3dat_unref(ls123);

	s3dat_unref(ls124);
	s3dat_unref(ls125);

	s3dat_unref(ls126);
	s3dat_unref(ls127);

	s3dat_unref(ls128);
	s3dat_unref(ls129);

	s3dat_unref(ls130);
	s3dat_unref(ls131);

	s3dat_unref(ls132);
	s3dat_unref(ls133);

	s3dat_unref(ls134);
	s3dat_unref(ls135);

	s3dat_unref(ls136);
	s3dat_unref(ls137);

	s3dat_unref(ls138);
	s3dat_unref(ls139);

	s3dat_unref(ls140);
	s3dat_unref(ls141);

	s3dat_unref(ls142);
	s3dat_unref(ls143);

	s3dat_unref(ls144);
	s3dat_unref(ls145);

	s3dat_unref(ls146);
	s3dat_unref(ls147);

	s3dat_unref(ls148);
	s3dat_unref(ls149);

	s3dat_unref(ls150);
	s3dat_unref(ls151);

	s3dat_unref(ls156);
	s3dat_unref(ls157);

	s3dat_unref(ls158);
	s3dat_unref(ls159);

	s3dat_unref(ls160);
	s3dat_unref(ls161);

	s3dat_unref(ls162);
	s3dat_unref(ls163);

	s3dat_unref(ls164);
	s3dat_unref(ls165);

	s3dat_unref(ls166);
	s3dat_unref(ls167);

	s3dat_unref(ls201);
	s3dat_unref(ls202);

	s3dat_unref(ls203);
	s3dat_unref(ls204);

	s3dat_unref(ls205);
	s3dat_unref(ls206);

	s3dat_unref(ls207);
	s3dat_unref(ls208);

	s3dat_unref(ls209);
	s3dat_unref(ls210);

	s3dat_unref(ls211);
	s3dat_unref(ls212);
	if(fields) s3util_free_func(&map->memset, fields);
	s3map_delete(map);

	glfwTerminate();
	return 0;
}
