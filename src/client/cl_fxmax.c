/*
Copyright (C) 1997-2001 Id Software, Inc.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/
// cl_fx.c -- entity effects parsing and management

#include "client.h"

void CL_ParticleBulletDecal (vec3_t org, vec3_t dir, float size);
void CL_LogoutEffect (vec3_t org, int type);
void CL_ItemRespawnParticles (vec3_t org);

void CL_GunSmokeEffect (vec3_t org, vec3_t dir);

trace_t CL_Trace (vec3_t start, vec3_t end, float size, int contentmask);
static vec3_t avelocities [NUMVERTEXNORMALS];

extern	struct model_s	*cl_mod_smoke;
extern	struct model_s	*cl_mod_flash;

//here i convert old 256 color to RGB -- hax0r l337
const byte default_pal[768] =
{
0,0,0,15,15,15,31,31,31,47,47,47,63,63,63,75,75,75,91,91,91,107,107,107,123,123,123,139,139,139,155,155,155,171,171,171,187,187,187,203,203,203,219,219,219,235,235,235,99,75,35,91,67,31,83,63,31,79,59,27,71,55,27,63,47,
23,59,43,23,51,39,19,47,35,19,43,31,19,39,27,15,35,23,15,27,19,11,23,15,11,19,15,7,15,11,7,95,95,111,91,91,103,91,83,95,87,79,91,83,75,83,79,71,75,71,63,67,63,59,59,59,55,55,51,47,47,47,43,43,39,
39,39,35,35,35,27,27,27,23,23,23,19,19,19,143,119,83,123,99,67,115,91,59,103,79,47,207,151,75,167,123,59,139,103,47,111,83,39,235,159,39,203,139,35,175,119,31,147,99,27,119,79,23,91,59,15,63,39,11,35,23,7,167,59,43,
159,47,35,151,43,27,139,39,19,127,31,15,115,23,11,103,23,7,87,19,0,75,15,0,67,15,0,59,15,0,51,11,0,43,11,0,35,11,0,27,7,0,19,7,0,123,95,75,115,87,67,107,83,63,103,79,59,95,71,55,87,67,51,83,63,
47,75,55,43,67,51,39,63,47,35,55,39,27,47,35,23,39,27,19,31,23,15,23,15,11,15,11,7,111,59,23,95,55,23,83,47,23,67,43,23,55,35,19,39,27,15,27,19,11,15,11,7,179,91,79,191,123,111,203,155,147,215,187,183,203,
215,223,179,199,211,159,183,195,135,167,183,115,151,167,91,135,155,71,119,139,47,103,127,23,83,111,19,75,103,15,67,91,11,63,83,7,55,75,7,47,63,7,39,51,0,31,43,0,23,31,0,15,19,0,7,11,0,0,0,139,87,87,131,79,79,
123,71,71,115,67,67,107,59,59,99,51,51,91,47,47,87,43,43,75,35,35,63,31,31,51,27,27,43,19,19,31,15,15,19,11,11,11,7,7,0,0,0,151,159,123,143,151,115,135,139,107,127,131,99,119,123,95,115,115,87,107,107,79,99,99,
71,91,91,67,79,79,59,67,67,51,55,55,43,47,47,35,35,35,27,23,23,19,15,15,11,159,75,63,147,67,55,139,59,47,127,55,39,119,47,35,107,43,27,99,35,23,87,31,19,79,27,15,67,23,11,55,19,11,43,15,7,31,11,7,23,
7,0,11,0,0,0,0,0,119,123,207,111,115,195,103,107,183,99,99,167,91,91,155,83,87,143,75,79,127,71,71,115,63,63,103,55,55,87,47,47,75,39,39,63,35,31,47,27,23,35,19,15,23,11,7,7,155,171,123,143,159,111,135,151,99,
123,139,87,115,131,75,103,119,67,95,111,59,87,103,51,75,91,39,63,79,27,55,67,19,47,59,11,35,47,7,27,35,0,19,23,0,11,15,0,0,255,0,35,231,15,63,211,27,83,187,39,95,167,47,95,143,51,95,123,51,255,255,255,255,255,
211,255,255,167,255,255,127,255,255,83,255,255,39,255,235,31,255,215,23,255,191,15,255,171,7,255,147,0,239,127,0,227,107,0,211,87,0,199,71,0,183,59,0,171,43,0,155,31,0,143,23,0,127,15,0,115,7,0,95,0,0,71,0,0,47,
0,0,27,0,0,239,0,0,55,55,255,255,0,0,0,0,255,43,43,35,27,27,23,19,19,15,235,151,127,195,115,83,159,87,51,123,63,27,235,211,199,199,171,155,167,139,119,135,107,87,159,91,83
};

//this initializes all particle images - mods play with this...
void SetParticleImages (void)
{
	//tgas
	re.SetParticlePicture(particle_generic,		"particles/basic.tga");
	re.SetParticlePicture(particle_smoke,		"particles/smoke.tga");
	re.SetParticlePicture(particle_blood,		"particles/blood.tga");
	re.SetParticlePicture(particle_blooddrop,	"particles/blood_drop.tga");
	re.SetParticlePicture(particle_blooddrip,	"particles/blood_drip.tga");
	re.SetParticlePicture(particle_redblood,	"particles/blood_red.tga");
	re.SetParticlePicture(particle_bubble,		"particles/bubble.tga");
	re.SetParticlePicture(particle_lensflare,	"particles/lensflare.tga");
	re.SetParticlePicture(particle_inferno,		"particles/inferno.tga");
	re.SetParticlePicture(particle_footprint,	"particles/footprint.tga");
	re.SetParticlePicture(particle_blaster,		"particles/blaster.tga");

	//jpgs
	re.SetParticlePicture(particle_shield,		"particles/shield.jpg");
	re.SetParticlePicture(particle_beam,		"particles/beam.jpg");
	re.SetParticlePicture(particle_lightning,	"particles/lightning.jpg");
	re.SetParticlePicture(particle_lightflare,	"particles/lightflare.jpg");

	//animations
		//explosion
		re.SetParticlePicture(particle_rexplosion1,	"particles/r_explod_1.tga");
		re.SetParticlePicture(particle_rexplosion2,	"particles/r_explod_2.tga");
		re.SetParticlePicture(particle_rexplosion3,	"particles/r_explod_3.tga");
		re.SetParticlePicture(particle_rexplosion4,	"particles/r_explod_4.tga");
		re.SetParticlePicture(particle_rexplosion5,	"particles/r_explod_5.tga");
		re.SetParticlePicture(particle_rexplosion6,	"particles/r_explod_6.tga");
		re.SetParticlePicture(particle_rexplosion7,	"particles/r_explod_7.tga");
		
		re.SetParticlePicture(particle_dexplosion1,	"particles/d_explod_1.tga");
		re.SetParticlePicture(particle_dexplosion2,	"particles/d_explod_2.tga");
		re.SetParticlePicture(particle_dexplosion3,	"particles/d_explod_3.tga");

}

void pRotateThink (cparticle_t *p, vec3_t org, vec3_t angle, float *alpha, float *size, int *image, float *time);

int	color8red (int color8)
{ 
	return (default_pal[color8*3+0]);
}
int	color8green (int color8)
{
	return (default_pal[color8*3+1]);;
}
int	color8blue (int color8)
{
	return (default_pal[color8*3+2]);;
}

void vectoanglerolled (vec3_t value1, float angleyaw, vec3_t angles)
{
	float	forward, yaw, pitch;

	yaw = (int) (atan2(value1[1], value1[0]) * 180 / M_PI);
	forward = sqrt (value1[0]*value1[0] + value1[1]*value1[1]);
	pitch = (int) (atan2(value1[2], forward) * 180 / M_PI);

	if (pitch < 0)
		pitch += 360;

	angles[PITCH] = -pitch;
	angles[YAW] =  yaw;
	angles[ROLL] = - angleyaw;
}

void ClipCam (vec3_t start, vec3_t end, vec3_t newpos)
{
	int i;

	trace_t tr = CL_Trace (start, end, 5, 1);
	for (i=0;i<3;i++)
		newpos[i]=tr.endpos[i];
}

/*
==============================================================

LIGHT STYLE MANAGEMENT

==============================================================
*/

typedef struct
{
	int		length;
	float	value[3];
	float	map[MAX_QPATH];
} clightstyle_t;

clightstyle_t	cl_lightstyle[MAX_LIGHTSTYLES];
int			lastofs;

/*
================
CL_ClearLightStyles
================
*/
void CL_ClearLightStyles (void)
{
	memset (cl_lightstyle, 0, sizeof(cl_lightstyle));
	lastofs = -1;
}

/*
================
CL_RunLightStyles
================
*/
void CL_RunLightStyles (void)
{
	int		ofs;
	int		i;
	clightstyle_t	*ls;

	ofs = cl.time / 100;
	if (ofs == lastofs)
		return;
	lastofs = ofs;

	for (i=0,ls=cl_lightstyle ; i<MAX_LIGHTSTYLES ; i++, ls++)
	{
		if (!ls->length)
		{
			ls->value[0] = ls->value[1] = ls->value[2] = 1.0;
			continue;
		}
		if (ls->length == 1)
			ls->value[0] = ls->value[1] = ls->value[2] = ls->map[0];
		else
			ls->value[0] = ls->value[1] = ls->value[2] = ls->map[ofs%ls->length];
	}
}


void CL_SetLightstyle (int i)
{
	char	*s;
	int		j, k;

	s = cl.configstrings[i+CS_LIGHTS];

	j = strlen (s);
	if (j >= MAX_QPATH)
		Com_Error (ERR_DROP, "svc_lightstyle length=%i", j);

	cl_lightstyle[i].length = j;

	for (k=0 ; k<j ; k++)
		cl_lightstyle[i].map[k] = (float)(s[k]-'a')/(float)('m'-'a');
}

/*
================
CL_AddLightStyles
================
*/
void CL_AddLightStyles (void)
{
	int		i;
	clightstyle_t	*ls;

	for (i=0,ls=cl_lightstyle ; i<MAX_LIGHTSTYLES ; i++, ls++)
		V_AddLightStyle (i, ls->value[0], ls->value[1], ls->value[2]);
}

/*
==============================================================

DLIGHT MANAGEMENT

==============================================================
*/

cdlight_t		cl_dlights[MAX_DLIGHTS];

/*
================
CL_ClearDlights
================
*/
void CL_ClearDlights (void)
{
	memset (cl_dlights, 0, sizeof(cl_dlights));
}

/*
===============
CL_AllocDlight

===============
*/
cdlight_t *CL_AllocDlight (int key)
{
	int		i;
	cdlight_t	*dl;

// first look for an exact key match
	if (key)
	{
		dl = cl_dlights;
		for (i=0 ; i<MAX_DLIGHTS ; i++, dl++)
		{
			if (dl->key == key)
			{
				memset (dl, 0, sizeof(*dl));
				dl->key = key;
				return dl;
			}
		}
	}

// then look for anything else
	dl = cl_dlights;
	for (i=0 ; i<MAX_DLIGHTS ; i++, dl++)
	{
		if (dl->die < cl.time)
		{
			memset (dl, 0, sizeof(*dl));
			dl->key = key;
			return dl;
		}
	}

	dl = &cl_dlights[0];
	memset (dl, 0, sizeof(*dl));
	dl->key = key;
	return dl;
}

cdlight_t *CL_AllocFreeDlight ()
{
	int		i;
	cdlight_t	*dl;

// then look for anything else
	dl = cl_dlights;
	for (i=0 ; i<MAX_DLIGHTS ; i++, dl++)
	{
		if (dl->die < cl.time)
		{
			memset (dl, 0, sizeof(*dl));
			dl->key = -1;
			return dl;
		}
	}

	dl = &cl_dlights[0];
	memset (dl, 0, sizeof(*dl));
	dl->key = -1;
	return dl;
}

/*
===============
CL_NewDlight
===============
*/
void CL_NewDlight (float x, float y, float z, float radius, float time)
{
	cdlight_t	*dl;

	dl = CL_AllocFreeDlight ();
	dl->origin[0] = x;
	dl->origin[1] = y;
	dl->origin[2] = z;
	dl->radius = radius;
	dl->die = cl.time + time;
}


/*
===============
CL_RunDLights

===============
*/
void CL_RunDLights (void)
{
	int			i;
	cdlight_t	*dl;

	dl = cl_dlights;
	for (i=0 ; i<MAX_DLIGHTS ; i++, dl++)
	{
		if (!dl->radius)
			continue;
		
		if (dl->die < cl.time)
		{
			dl->radius = 0;
			return;
		}
		dl->radius -= cls.frametime*dl->decay;
		if (dl->radius < 0)
			dl->radius = 0;
	}
}

/*
==============
CL_ParseMuzzleFlash
==============
*/
void CL_ParseMuzzleFlash (void)
{
	vec3_t		fv, rv;
	cdlight_t	*dl;
	int			i, weapon;
	centity_t	*pl;
	int			silenced;
	float		volume;
	char		soundname[64];

	i = MSG_ReadShort (&net_message);
	if (i < 1 || i >= MAX_EDICTS)
		Com_Error (ERR_DROP, "CL_ParseMuzzleFlash: bad entity");

	weapon = MSG_ReadByte (&net_message);
	silenced = weapon & MZ_SILENCED;
	weapon &= ~MZ_SILENCED;

	pl = &cl_entities[i];

	dl = CL_AllocDlight (i);
	VectorCopy (pl->current.origin,  dl->origin);
	AngleVectors (pl->current.angles, fv, rv, NULL);
	VectorMA (dl->origin, 18, fv, dl->origin);
	VectorMA (dl->origin, 16, rv, dl->origin);
	if (silenced)
		dl->radius = 100 + (rand()&31);
	else
		dl->radius = 200 + (rand()&31);
	dl->minlight = 32;
	dl->die = cl.time; // + 0.1;

	if (silenced)
		volume = 0.2;
	else
		volume = 1;

	switch (weapon)
	{
	case MZ_BLASTER:
		dl->color[0] = 1;dl->color[1] = 1;dl->color[2] = 0;
		S_StartSound (NULL, i, CHAN_WEAPON, S_RegisterSound("weapons/blastf1a.wav"), volume, ATTN_NORM, 0);
		break;
	case MZ_BLUEHYPERBLASTER:
		dl->color[0] = 0;dl->color[1] = 0;dl->color[2] = 1;
		S_StartSound (NULL, i, CHAN_WEAPON, S_RegisterSound("weapons/hyprbf1a.wav"), volume, ATTN_NORM, 0);
		break;
	case MZ_HYPERBLASTER:
		dl->color[0] = 1;dl->color[1] = 1;dl->color[2] = 0;
		S_StartSound (NULL, i, CHAN_WEAPON, S_RegisterSound("weapons/hyprbf1a.wav"), volume, ATTN_NORM, 0);
		break;
	case MZ_MACHINEGUN:
		dl->color[0] = 1;dl->color[1] = 1;dl->color[2] = 0.5;
		Com_sprintf(soundname, sizeof(soundname), "weapons/machgf%ib.wav", (rand() % 5) + 1);
		S_StartSound (NULL, i, CHAN_WEAPON, S_RegisterSound(soundname), volume, ATTN_NORM, 0);
		break;
	case MZ_SHOTGUN:
		dl->color[0] = 1;dl->color[1] = 1;dl->color[2] = 0.5;
		S_StartSound (NULL, i, CHAN_WEAPON, S_RegisterSound("weapons/shotgf1b.wav"), volume, ATTN_NORM, 0);
		S_StartSound (NULL, i, CHAN_AUTO,   S_RegisterSound("weapons/shotgr1b.wav"), volume, ATTN_NORM, 0.1);
		break;
	case MZ_SSHOTGUN:
		dl->color[0] = 1;dl->color[1] = 1;dl->color[2] = 0.5;
		S_StartSound (NULL, i, CHAN_WEAPON, S_RegisterSound("weapons/sshotf1b.wav"), volume, ATTN_NORM, 0);
		break;
	case MZ_CHAINGUN1:
		dl->radius = 200 + (rand()&31);
		dl->color[0] = 1;dl->color[1] = 0.5;dl->color[2] = 0.25;
		Com_sprintf(soundname, sizeof(soundname), "weapons/machgf%ib.wav", (rand() % 5) + 1);
		S_StartSound (NULL, i, CHAN_WEAPON, S_RegisterSound(soundname), volume, ATTN_NORM, 0);
		break;
	case MZ_CHAINGUN2:
		dl->radius = 225 + (rand()&31);
		dl->color[0] = 1;dl->color[1] = 0.5;dl->color[2] = 0.1;
		dl->die = cl.time  + 0.1;	// long delay
		Com_sprintf(soundname, sizeof(soundname), "weapons/machgf%ib.wav", (rand() % 5) + 1);
		S_StartSound (NULL, i, CHAN_WEAPON, S_RegisterSound(soundname), volume, ATTN_NORM, 0);
		Com_sprintf(soundname, sizeof(soundname), "weapons/machgf%ib.wav", (rand() % 5) + 1);
		S_StartSound (NULL, i, CHAN_WEAPON, S_RegisterSound(soundname), volume, ATTN_NORM, 0.05);
		break;
	case MZ_CHAINGUN3:
		dl->radius = 250 + (rand()&31);
		dl->color[0] = 1;dl->color[1] = 1;dl->color[2] = 0.25;
		dl->die = cl.time  + 0.1;	// long delay
		Com_sprintf(soundname, sizeof(soundname), "weapons/machgf%ib.wav", (rand() % 5) + 1);
		S_StartSound (NULL, i, CHAN_WEAPON, S_RegisterSound(soundname), volume, ATTN_NORM, 0);
		Com_sprintf(soundname, sizeof(soundname), "weapons/machgf%ib.wav", (rand() % 5) + 1);
		S_StartSound (NULL, i, CHAN_WEAPON, S_RegisterSound(soundname), volume, ATTN_NORM, 0.033);
		Com_sprintf(soundname, sizeof(soundname), "weapons/machgf%ib.wav", (rand() % 5) + 1);
		S_StartSound (NULL, i, CHAN_WEAPON, S_RegisterSound(soundname), volume, ATTN_NORM, 0.066);
		break;
	case MZ_RAILGUN:
		dl->color[0] = cl_railred->value/255;
		dl->color[1] = cl_railgreen->value/255;
		dl->color[2] = cl_railblue->value/255;
		dl->die += 10000;
		dl->decay = 100;
		S_StartSound (NULL, i, CHAN_WEAPON, S_RegisterSound("weapons/railgf1a.wav"), volume, ATTN_NORM, 0);
		break;
	case MZ_ROCKET:
		dl->color[0] = 1;dl->color[1] = 0.5;dl->color[2] = 0.2;
		S_StartSound (NULL, i, CHAN_WEAPON, S_RegisterSound("weapons/rocklf1a.wav"), volume, ATTN_NORM, 0);
		S_StartSound (NULL, i, CHAN_AUTO,   S_RegisterSound("weapons/rocklr1b.wav"), volume, ATTN_NORM, 0.1);
		break;
	case MZ_GRENADE:
		dl->color[0] = 1;dl->color[1] = 0.5;dl->color[2] = 0;
		S_StartSound (NULL, i, CHAN_WEAPON, S_RegisterSound("weapons/grenlf1a.wav"), volume, ATTN_NORM, 0);
		S_StartSound (NULL, i, CHAN_AUTO,   S_RegisterSound("weapons/grenlr1b.wav"), volume, ATTN_NORM, 0.1);
		break;
	case MZ_BFG:
		dl->color[0] = 0;dl->color[1] = 1;dl->color[2] = 0;
		S_StartSound (NULL, i, CHAN_WEAPON, S_RegisterSound("weapons/bfg__f1y.wav"), volume, ATTN_NORM, 0);
		break;

	case MZ_LOGIN:
		dl->color[0] = 0;dl->color[1] = 1; dl->color[2] = 0;
		dl->die = cl.time + 1.0;
		S_StartSound (NULL, i, CHAN_WEAPON, S_RegisterSound("weapons/grenlf1a.wav"), 1, ATTN_NORM, 0);
		CL_LogoutEffect (pl->current.origin, weapon);
		break;
	case MZ_LOGOUT:
		dl->color[0] = 1;dl->color[1] = 0; dl->color[2] = 0;
		dl->die = cl.time + 1.0;
		S_StartSound (NULL, i, CHAN_WEAPON, S_RegisterSound("weapons/grenlf1a.wav"), 1, ATTN_NORM, 0);
		CL_LogoutEffect (pl->current.origin, weapon);
		break;
	case MZ_RESPAWN:
		dl->color[0] = 1;dl->color[1] = 1; dl->color[2] = 0;
		dl->die = cl.time + 1.0;
		S_StartSound (NULL, i, CHAN_WEAPON, S_RegisterSound("weapons/grenlf1a.wav"), 1, ATTN_NORM, 0);
		CL_LogoutEffect (pl->current.origin, weapon);
		break;
	// RAFAEL
	case MZ_PHALANX:
		dl->color[0] = 1;dl->color[1] = 0.5; dl->color[2] = 0.5;
		S_StartSound (NULL, i, CHAN_WEAPON, S_RegisterSound("weapons/plasshot.wav"), volume, ATTN_NORM, 0);
		break;
	// RAFAEL
	case MZ_IONRIPPER:	
		dl->color[0] = 1;dl->color[1] = 0.5; dl->color[2] = 0.5;
		S_StartSound (NULL, i, CHAN_WEAPON, S_RegisterSound("weapons/rippfire.wav"), volume, ATTN_NORM, 0);
		break;

// ======================
// PGM
	case MZ_ETF_RIFLE:
		dl->color[0] = 0.9;dl->color[1] = 0.7;dl->color[2] = 0;
		S_StartSound (NULL, i, CHAN_WEAPON, S_RegisterSound("weapons/nail1.wav"), volume, ATTN_NORM, 0);
		break;
	case MZ_SHOTGUN2:
		dl->color[0] = 1;dl->color[1] = 1;dl->color[2] = 0;
		S_StartSound (NULL, i, CHAN_WEAPON, S_RegisterSound("weapons/shotg2.wav"), volume, ATTN_NORM, 0);
		break;
	case MZ_HEATBEAM:
		dl->color[0] = 1;dl->color[1] = 1;dl->color[2] = 0;
		dl->die = cl.time + 100;
//		S_StartSound (NULL, i, CHAN_WEAPON, S_RegisterSound("weapons/bfg__l1a.wav"), volume, ATTN_NORM, 0);
		break;
	case MZ_BLASTER2:
		dl->color[0] = 0;dl->color[1] = 1;dl->color[2] = 0;
		// FIXME - different sound for blaster2 ??
		S_StartSound (NULL, i, CHAN_WEAPON, S_RegisterSound("weapons/blastf1a.wav"), volume, ATTN_NORM, 0);
		break;
	case MZ_TRACKER:
		dl->color[0] = -1;dl->color[1] = -1;dl->color[2] = -1;
		S_StartSound (NULL, i, CHAN_WEAPON, S_RegisterSound("weapons/disint2.wav"), volume, ATTN_NORM, 0);
		break;		
	case MZ_NUKE1:
		dl->color[0] = 1;dl->color[1] = 0;dl->color[2] = 0;
		dl->die = cl.time + 100;
		break;
	case MZ_NUKE2:
		dl->color[0] = 1;dl->color[1] = 1;dl->color[2] = 0;
		dl->die = cl.time + 100;
		break;
	case MZ_NUKE4:
		dl->color[0] = 0;dl->color[1] = 0;dl->color[2] = 1;
		dl->die = cl.time + 100;
		break;
	case MZ_NUKE8:
		dl->color[0] = 0;dl->color[1] = 1;dl->color[2] = 1;
		dl->die = cl.time + 100;
		break;
// PGM
// ======================
	}
}


/*
==============
CL_ParseMuzzleFlash2
==============
*/
void CL_ParseMuzzleFlash2 (void) 
{
	int			ent;
	vec3_t		origin;
	int			flash_number;
	cdlight_t	*dl;
	vec3_t		forward, right;
	char		soundname[64];

	ent = MSG_ReadShort (&net_message);
	if (ent < 1 || ent >= MAX_EDICTS)
		Com_Error (ERR_DROP, "CL_ParseMuzzleFlash2: bad entity");

	flash_number = MSG_ReadByte (&net_message);

	// locate the origin
	AngleVectors (cl_entities[ent].current.angles, forward, right, NULL);
	origin[0] = cl_entities[ent].current.origin[0] + forward[0] * monster_flash_offset[flash_number][0] + right[0] * monster_flash_offset[flash_number][1];
	origin[1] = cl_entities[ent].current.origin[1] + forward[1] * monster_flash_offset[flash_number][0] + right[1] * monster_flash_offset[flash_number][1];
	origin[2] = cl_entities[ent].current.origin[2] + forward[2] * monster_flash_offset[flash_number][0] + right[2] * monster_flash_offset[flash_number][1] + monster_flash_offset[flash_number][2];

	dl = CL_AllocDlight (ent);
	VectorCopy (origin,  dl->origin);
	dl->radius = 200 + (rand()&31);
	dl->minlight = 32;
	dl->die = cl.time;	// + 0.1;

	switch (flash_number)
	{
	case MZ2_INFANTRY_MACHINEGUN_1:
	case MZ2_INFANTRY_MACHINEGUN_2:
	case MZ2_INFANTRY_MACHINEGUN_3:
	case MZ2_INFANTRY_MACHINEGUN_4:
	case MZ2_INFANTRY_MACHINEGUN_5:
	case MZ2_INFANTRY_MACHINEGUN_6:
	case MZ2_INFANTRY_MACHINEGUN_7:
	case MZ2_INFANTRY_MACHINEGUN_8:
	case MZ2_INFANTRY_MACHINEGUN_9:
	case MZ2_INFANTRY_MACHINEGUN_10:
	case MZ2_INFANTRY_MACHINEGUN_11:
	case MZ2_INFANTRY_MACHINEGUN_12:
	case MZ2_INFANTRY_MACHINEGUN_13:
		dl->color[0] = 1;dl->color[1] = 1;dl->color[2] = 0;
		CL_GunSmokeEffect (origin, vec3_origin);
		//CL_ParticleEffect (origin, vec3_origin, 0, 40);
		//CL_SmokeAndFlash(origin);
		S_StartSound (NULL, ent, CHAN_WEAPON, S_RegisterSound("infantry/infatck1.wav"), 1, ATTN_NORM, 0);
		break;

	case MZ2_SOLDIER_MACHINEGUN_1:
	case MZ2_SOLDIER_MACHINEGUN_2:
	case MZ2_SOLDIER_MACHINEGUN_3:
	case MZ2_SOLDIER_MACHINEGUN_4:
	case MZ2_SOLDIER_MACHINEGUN_5:
	case MZ2_SOLDIER_MACHINEGUN_6:
	case MZ2_SOLDIER_MACHINEGUN_7:
	case MZ2_SOLDIER_MACHINEGUN_8:
		dl->color[0] = 1;dl->color[1] = 1;dl->color[2] = 0;
		CL_GunSmokeEffect (origin, vec3_origin);
		//CL_ParticleEffect (origin, vec3_origin, 0, 40);
		//CL_SmokeAndFlash(origin);
		S_StartSound (NULL, ent, CHAN_WEAPON, S_RegisterSound("soldier/solatck3.wav"), 1, ATTN_NORM, 0);
		break;

	case MZ2_GUNNER_MACHINEGUN_1:
	case MZ2_GUNNER_MACHINEGUN_2:
	case MZ2_GUNNER_MACHINEGUN_3:
	case MZ2_GUNNER_MACHINEGUN_4:
	case MZ2_GUNNER_MACHINEGUN_5:
	case MZ2_GUNNER_MACHINEGUN_6:
	case MZ2_GUNNER_MACHINEGUN_7:
	case MZ2_GUNNER_MACHINEGUN_8:
		dl->color[0] = 1;dl->color[1] = 1;dl->color[2] = 0;
		CL_GunSmokeEffect (origin, vec3_origin);
		//CL_ParticleEffect (origin, vec3_origin, 0, 40);
		//CL_SmokeAndFlash(origin);
		S_StartSound (NULL, ent, CHAN_WEAPON, S_RegisterSound("gunner/gunatck2.wav"), 1, ATTN_NORM, 0);
		break;

	case MZ2_ACTOR_MACHINEGUN_1:
	case MZ2_SUPERTANK_MACHINEGUN_1:
	case MZ2_SUPERTANK_MACHINEGUN_2:
	case MZ2_SUPERTANK_MACHINEGUN_3:
	case MZ2_SUPERTANK_MACHINEGUN_4:
	case MZ2_SUPERTANK_MACHINEGUN_5:
	case MZ2_SUPERTANK_MACHINEGUN_6:
	case MZ2_TURRET_MACHINEGUN:			// PGM
		dl->color[0] = 1;dl->color[1] = 1;dl->color[2] = 0;

		CL_GunSmokeEffect (origin, vec3_origin);
		//CL_ParticleEffect (origin, vec3_origin, 0, 40);
		//CL_SmokeAndFlash(origin);
		S_StartSound (NULL, ent, CHAN_WEAPON, S_RegisterSound("infantry/infatck1.wav"), 1, ATTN_NORM, 0);
		break;

	case MZ2_BOSS2_MACHINEGUN_L1:
	case MZ2_BOSS2_MACHINEGUN_L2:
	case MZ2_BOSS2_MACHINEGUN_L3:
	case MZ2_BOSS2_MACHINEGUN_L4:
	case MZ2_BOSS2_MACHINEGUN_L5:
	case MZ2_CARRIER_MACHINEGUN_L1:		// PMM
	case MZ2_CARRIER_MACHINEGUN_L2:		// PMM
		dl->color[0] = 1;dl->color[1] = 1;dl->color[2] = 0;

		CL_GunSmokeEffect (origin, vec3_origin);
		//CL_ParticleEffect (origin, vec3_origin, 0, 40);
		//CL_SmokeAndFlash(origin);
		S_StartSound (NULL, ent, CHAN_WEAPON, S_RegisterSound("infantry/infatck1.wav"), 1, ATTN_NONE, 0);
		break;

	case MZ2_SOLDIER_BLASTER_1:
	case MZ2_SOLDIER_BLASTER_2:
	case MZ2_SOLDIER_BLASTER_3:
	case MZ2_SOLDIER_BLASTER_4:
	case MZ2_SOLDIER_BLASTER_5:
	case MZ2_SOLDIER_BLASTER_6:
	case MZ2_SOLDIER_BLASTER_7:
	case MZ2_SOLDIER_BLASTER_8:
	case MZ2_TURRET_BLASTER:			// PGM
		dl->color[0] = 1;dl->color[1] = 1;dl->color[2] = 0;
		S_StartSound (NULL, ent, CHAN_WEAPON, S_RegisterSound("soldier/solatck2.wav"), 1, ATTN_NORM, 0);
		break;

	case MZ2_FLYER_BLASTER_1:
	case MZ2_FLYER_BLASTER_2:
		dl->color[0] = 1;dl->color[1] = 1;dl->color[2] = 0;
		S_StartSound (NULL, ent, CHAN_WEAPON, S_RegisterSound("flyer/flyatck3.wav"), 1, ATTN_NORM, 0);
		break;

	case MZ2_MEDIC_BLASTER_1:
		dl->color[0] = 1;dl->color[1] = 1;dl->color[2] = 0;
		S_StartSound (NULL, ent, CHAN_WEAPON, S_RegisterSound("medic/medatck1.wav"), 1, ATTN_NORM, 0);
		break;

	case MZ2_HOVER_BLASTER_1:
		dl->color[0] = 1;dl->color[1] = 1;dl->color[2] = 0;
		S_StartSound (NULL, ent, CHAN_WEAPON, S_RegisterSound("hover/hovatck1.wav"), 1, ATTN_NORM, 0);
		break;

	case MZ2_FLOAT_BLASTER_1:
		dl->color[0] = 1;dl->color[1] = 1;dl->color[2] = 0;
		S_StartSound (NULL, ent, CHAN_WEAPON, S_RegisterSound("floater/fltatck1.wav"), 1, ATTN_NORM, 0);
		break;

	case MZ2_SOLDIER_SHOTGUN_1:
	case MZ2_SOLDIER_SHOTGUN_2:
	case MZ2_SOLDIER_SHOTGUN_3:
	case MZ2_SOLDIER_SHOTGUN_4:
	case MZ2_SOLDIER_SHOTGUN_5:
	case MZ2_SOLDIER_SHOTGUN_6:
	case MZ2_SOLDIER_SHOTGUN_7:
	case MZ2_SOLDIER_SHOTGUN_8:
		dl->color[0] = 1;dl->color[1] = 1;dl->color[2] = 0;
		CL_GunSmokeEffect (origin, vec3_origin);
		//CL_SmokeAndFlash(origin);
		S_StartSound (NULL, ent, CHAN_WEAPON, S_RegisterSound("soldier/solatck1.wav"), 1, ATTN_NORM, 0);
		break;

	case MZ2_TANK_BLASTER_1:
	case MZ2_TANK_BLASTER_2:
	case MZ2_TANK_BLASTER_3:
		dl->color[0] = 1;dl->color[1] = 1;dl->color[2] = 0;
		S_StartSound (NULL, ent, CHAN_WEAPON, S_RegisterSound("tank/tnkatck3.wav"), 1, ATTN_NORM, 0);
		break;

	case MZ2_TANK_MACHINEGUN_1:
	case MZ2_TANK_MACHINEGUN_2:
	case MZ2_TANK_MACHINEGUN_3:
	case MZ2_TANK_MACHINEGUN_4:
	case MZ2_TANK_MACHINEGUN_5:
	case MZ2_TANK_MACHINEGUN_6:
	case MZ2_TANK_MACHINEGUN_7:
	case MZ2_TANK_MACHINEGUN_8:
	case MZ2_TANK_MACHINEGUN_9:
	case MZ2_TANK_MACHINEGUN_10:
	case MZ2_TANK_MACHINEGUN_11:
	case MZ2_TANK_MACHINEGUN_12:
	case MZ2_TANK_MACHINEGUN_13:
	case MZ2_TANK_MACHINEGUN_14:
	case MZ2_TANK_MACHINEGUN_15:
	case MZ2_TANK_MACHINEGUN_16:
	case MZ2_TANK_MACHINEGUN_17:
	case MZ2_TANK_MACHINEGUN_18:
	case MZ2_TANK_MACHINEGUN_19:
		dl->color[0] = 1;dl->color[1] = 1;dl->color[2] = 0;
		CL_GunSmokeEffect (origin, vec3_origin);
		//CL_ParticleEffect (origin, vec3_origin, 0, 40);
		//CL_SmokeAndFlash(origin);
		Com_sprintf(soundname, sizeof(soundname), "tank/tnkatk2%c.wav", 'a' + rand() % 5);
		S_StartSound (NULL, ent, CHAN_WEAPON, S_RegisterSound(soundname), 1, ATTN_NORM, 0);
		break;

	case MZ2_CHICK_ROCKET_1:
	case MZ2_TURRET_ROCKET:			// PGM
		dl->color[0] = 1;dl->color[1] = 0.5;dl->color[2] = 0.2;
		S_StartSound (NULL, ent, CHAN_WEAPON, S_RegisterSound("chick/chkatck2.wav"), 1, ATTN_NORM, 0);
		break;

	case MZ2_TANK_ROCKET_1:
	case MZ2_TANK_ROCKET_2:
	case MZ2_TANK_ROCKET_3:
		dl->color[0] = 1;dl->color[1] = 0.5;dl->color[2] = 0.2;
		S_StartSound (NULL, ent, CHAN_WEAPON, S_RegisterSound("tank/tnkatck1.wav"), 1, ATTN_NORM, 0);
		break;

	case MZ2_SUPERTANK_ROCKET_1:
	case MZ2_SUPERTANK_ROCKET_2:
	case MZ2_SUPERTANK_ROCKET_3:
	case MZ2_BOSS2_ROCKET_1:
	case MZ2_BOSS2_ROCKET_2:
	case MZ2_BOSS2_ROCKET_3:
	case MZ2_BOSS2_ROCKET_4:
	case MZ2_CARRIER_ROCKET_1:
//	case MZ2_CARRIER_ROCKET_2:
//	case MZ2_CARRIER_ROCKET_3:
//	case MZ2_CARRIER_ROCKET_4:
		dl->color[0] = 1;dl->color[1] = 0.5;dl->color[2] = 0.2;
		S_StartSound (NULL, ent, CHAN_WEAPON, S_RegisterSound("tank/rocket.wav"), 1, ATTN_NORM, 0);
		break;

	case MZ2_GUNNER_GRENADE_1:
	case MZ2_GUNNER_GRENADE_2:
	case MZ2_GUNNER_GRENADE_3:
	case MZ2_GUNNER_GRENADE_4:
		dl->color[0] = 1;dl->color[1] = 0.5;dl->color[2] = 0;
		S_StartSound (NULL, ent, CHAN_WEAPON, S_RegisterSound("gunner/gunatck3.wav"), 1, ATTN_NORM, 0);
		break;

	case MZ2_GLADIATOR_RAILGUN_1:
	// PMM
	case MZ2_CARRIER_RAILGUN:
	case MZ2_WIDOW_RAIL:
	// pmm
		dl->color[0] = 0.5;dl->color[1] = 0.5;dl->color[2] = 1.0;
		break;

// --- Xian's shit starts ---
	case MZ2_MAKRON_BFG:
		dl->color[0] = 0.5;dl->color[1] = 1 ;dl->color[2] = 0.5;
		//S_StartSound (NULL, ent, CHAN_WEAPON, S_RegisterSound("makron/bfg_fire.wav"), 1, ATTN_NORM, 0);
		break;

	case MZ2_MAKRON_BLASTER_1:
	case MZ2_MAKRON_BLASTER_2:
	case MZ2_MAKRON_BLASTER_3:
	case MZ2_MAKRON_BLASTER_4:
	case MZ2_MAKRON_BLASTER_5:
	case MZ2_MAKRON_BLASTER_6:
	case MZ2_MAKRON_BLASTER_7:
	case MZ2_MAKRON_BLASTER_8:
	case MZ2_MAKRON_BLASTER_9:
	case MZ2_MAKRON_BLASTER_10:
	case MZ2_MAKRON_BLASTER_11:
	case MZ2_MAKRON_BLASTER_12:
	case MZ2_MAKRON_BLASTER_13:
	case MZ2_MAKRON_BLASTER_14:
	case MZ2_MAKRON_BLASTER_15:
	case MZ2_MAKRON_BLASTER_16:
	case MZ2_MAKRON_BLASTER_17:
		dl->color[0] = 1;dl->color[1] = 1;dl->color[2] = 0;
		S_StartSound (NULL, ent, CHAN_WEAPON, S_RegisterSound("makron/blaster.wav"), 1, ATTN_NORM, 0);
		break;
	
	case MZ2_JORG_MACHINEGUN_L1:
	case MZ2_JORG_MACHINEGUN_L2:
	case MZ2_JORG_MACHINEGUN_L3:
	case MZ2_JORG_MACHINEGUN_L4:
	case MZ2_JORG_MACHINEGUN_L5:
	case MZ2_JORG_MACHINEGUN_L6:
		dl->color[0] = 1;dl->color[1] = 1;dl->color[2] = 0;
		CL_GunSmokeEffect (origin, vec3_origin);
		//CL_ParticleEffect (origin, vec3_origin, 0, 40);
		//CL_SmokeAndFlash(origin);
		S_StartSound (NULL, ent, CHAN_WEAPON, S_RegisterSound("boss3/xfire.wav"), 1, ATTN_NORM, 0);
		break;

	case MZ2_JORG_MACHINEGUN_R1:
	case MZ2_JORG_MACHINEGUN_R2:
	case MZ2_JORG_MACHINEGUN_R3:
	case MZ2_JORG_MACHINEGUN_R4:
	case MZ2_JORG_MACHINEGUN_R5:
	case MZ2_JORG_MACHINEGUN_R6:
		dl->color[0] = 1;dl->color[1] = 1;dl->color[2] = 0;
		CL_GunSmokeEffect (origin, vec3_origin);
		//CL_ParticleEffect (origin, vec3_origin, 0, 40);
		//CL_SmokeAndFlash(origin);
		break;

	case MZ2_JORG_BFG_1:
		dl->color[0] = 0.5;dl->color[1] = 1 ;dl->color[2] = 0.5;
		break;

	case MZ2_BOSS2_MACHINEGUN_R1:
	case MZ2_BOSS2_MACHINEGUN_R2:
	case MZ2_BOSS2_MACHINEGUN_R3:
	case MZ2_BOSS2_MACHINEGUN_R4:
	case MZ2_BOSS2_MACHINEGUN_R5:
	case MZ2_CARRIER_MACHINEGUN_R1:			// PMM
	case MZ2_CARRIER_MACHINEGUN_R2:			// PMM

		dl->color[0] = 1;dl->color[1] = 1;dl->color[2] = 0;

		CL_GunSmokeEffect (origin, vec3_origin);
		//CL_ParticleEffect (origin, vec3_origin, 0, 40);
		//CL_SmokeAndFlash(origin);
		break;

// ======
// ROGUE
	case MZ2_STALKER_BLASTER:
	case MZ2_DAEDALUS_BLASTER:
	case MZ2_MEDIC_BLASTER_2:
	case MZ2_WIDOW_BLASTER:
	case MZ2_WIDOW_BLASTER_SWEEP1:
	case MZ2_WIDOW_BLASTER_SWEEP2:
	case MZ2_WIDOW_BLASTER_SWEEP3:
	case MZ2_WIDOW_BLASTER_SWEEP4:
	case MZ2_WIDOW_BLASTER_SWEEP5:
	case MZ2_WIDOW_BLASTER_SWEEP6:
	case MZ2_WIDOW_BLASTER_SWEEP7:
	case MZ2_WIDOW_BLASTER_SWEEP8:
	case MZ2_WIDOW_BLASTER_SWEEP9:
	case MZ2_WIDOW_BLASTER_100:
	case MZ2_WIDOW_BLASTER_90:
	case MZ2_WIDOW_BLASTER_80:
	case MZ2_WIDOW_BLASTER_70:
	case MZ2_WIDOW_BLASTER_60:
	case MZ2_WIDOW_BLASTER_50:
	case MZ2_WIDOW_BLASTER_40:
	case MZ2_WIDOW_BLASTER_30:
	case MZ2_WIDOW_BLASTER_20:
	case MZ2_WIDOW_BLASTER_10:
	case MZ2_WIDOW_BLASTER_0:
	case MZ2_WIDOW_BLASTER_10L:
	case MZ2_WIDOW_BLASTER_20L:
	case MZ2_WIDOW_BLASTER_30L:
	case MZ2_WIDOW_BLASTER_40L:
	case MZ2_WIDOW_BLASTER_50L:
	case MZ2_WIDOW_BLASTER_60L:
	case MZ2_WIDOW_BLASTER_70L:
	case MZ2_WIDOW_RUN_1:
	case MZ2_WIDOW_RUN_2:
	case MZ2_WIDOW_RUN_3:
	case MZ2_WIDOW_RUN_4:
	case MZ2_WIDOW_RUN_5:
	case MZ2_WIDOW_RUN_6:
	case MZ2_WIDOW_RUN_7:
	case MZ2_WIDOW_RUN_8:
		dl->color[0] = 0;dl->color[1] = 1;dl->color[2] = 0;
		S_StartSound (NULL, ent, CHAN_WEAPON, S_RegisterSound("tank/tnkatck3.wav"), 1, ATTN_NORM, 0);
		break;

	case MZ2_WIDOW_DISRUPTOR:
		dl->color[0] = -1;dl->color[1] = -1;dl->color[2] = -1;
		S_StartSound (NULL, ent, CHAN_WEAPON, S_RegisterSound("weapons/disint2.wav"), 1, ATTN_NORM, 0);
		break;

	case MZ2_WIDOW_PLASMABEAM:
	case MZ2_WIDOW2_BEAMER_1:
	case MZ2_WIDOW2_BEAMER_2:
	case MZ2_WIDOW2_BEAMER_3:
	case MZ2_WIDOW2_BEAMER_4:
	case MZ2_WIDOW2_BEAMER_5:
	case MZ2_WIDOW2_BEAM_SWEEP_1:
	case MZ2_WIDOW2_BEAM_SWEEP_2:
	case MZ2_WIDOW2_BEAM_SWEEP_3:
	case MZ2_WIDOW2_BEAM_SWEEP_4:
	case MZ2_WIDOW2_BEAM_SWEEP_5:
	case MZ2_WIDOW2_BEAM_SWEEP_6:
	case MZ2_WIDOW2_BEAM_SWEEP_7:
	case MZ2_WIDOW2_BEAM_SWEEP_8:
	case MZ2_WIDOW2_BEAM_SWEEP_9:
	case MZ2_WIDOW2_BEAM_SWEEP_10:
	case MZ2_WIDOW2_BEAM_SWEEP_11:
		dl->radius = 300 + (rand()&100);
		dl->color[0] = 1;dl->color[1] = 1;dl->color[2] = 0;
		dl->die = cl.time + 200;
		break;
// ROGUE
// ======

// --- Xian's shit ends ---

	}
}


/*
===============
CL_AddDLights

===============
*/
void CL_AddDLights (void)
{
	int			i;
	cdlight_t	*dl;

	dl = cl_dlights;

	for (i=0 ; i<MAX_DLIGHTS ; i++, dl++)
	{
		if (!dl->radius)
			continue;
		V_AddLight (dl->origin, dl->radius,
			dl->color[0], dl->color[1], dl->color[2]);
	}
}



/*
==============================================================

PARTICLE MANAGEMENT

==============================================================
*/

cparticle_t	*active_particles, *free_particles;

cparticle_t	particles[MAX_PARTICLES];
int			cl_numparticles = MAX_PARTICLES;

cparticle_t *setupParticle (
			float angle0,		float angle1,		float angle2,
			float org0,			float org1,			float org2,
			float vel0,			float vel1,			float vel2,
			float accel0,		float accel1,		float accel2,
			float color0,		float color1,		float color2,
			float colorvel0,	float colorvel1,	float colorvel2,
			float alpha,		float alphavel,
			float size,			float sizevel,			
			int	image,
			int flags,
			void (*think)(cparticle_t *p, vec3_t org, vec3_t angle, float *alpha, float *size, int *image, float *time),
			qboolean thinknext)
{
	int j;
	cparticle_t	*p = NULL;

	if (!free_particles)
		return NULL;
	p = free_particles;
	free_particles = p->next;
	p->next = active_particles;
	active_particles = p;

	p->start = p->time = cl.time;

	p->angle[0]=angle0;
	p->angle[1]=angle1;
	p->angle[2]=angle2;

	p->org[0]=org0;
	p->org[1]=org1;
	p->org[2]=org2;

	p->vel[0]=vel0;
	p->vel[1]=vel1;
	p->vel[2]=vel2;

	p->accel[0]=accel0;
	p->accel[1]=accel1;
	p->accel[2]=accel2;

	p->color[0]=color0;
	p->color[1]=color1;
	p->color[2]=color2;

	p->colorvel[0]=colorvel0;
	p->colorvel[1]=colorvel1;
	p->colorvel[2]=colorvel2;

	p->alpha=alpha;
	p->alphavel=alphavel;
	p->size=size;
	p->sizevel=sizevel;

	p->image=image;
	p->flags=flags;

	p->src_ent=0;
	p->dst_ent=0;

	if (think)
		p->think=think;
	else
		p->think=NULL;
	p->thinknext=thinknext;

	for (j=0;j<P_LIGHTS_MAX;j++)
	{
		cplight_t *plight = &p->lights[j];
		plight->isactive = false;
		plight->light = 0;
		plight->lightvel = 0;
		plight->lightcol[0] = 0;
		plight->lightcol[1] = 0;
		plight->lightcol[2] = 0;
	}

	return p;
}

void addParticleLight (cparticle_t *p,
				  float light, float lightvel,
				  float lcol0, float lcol1, float lcol2)
{
	int i;

	for (i=0; i<P_LIGHTS_MAX; i++)
	{
		cplight_t *plight = &p->lights[i];
		if (!plight->isactive)
		{
			plight->isactive = true;
			plight->light = light;
			plight->lightvel = lightvel;
			plight->lightcol[0] = lcol0;
			plight->lightcol[1] = lcol1;
			plight->lightcol[2] = lcol2;
			return;
		}
	}
}

/*
===============
CL_ClearParticles
===============
*/
void CL_ClearParticles (void)
{
	int		i;
	
	free_particles = &particles[0];
	active_particles = NULL;

	for (i=0 ;i<cl_numparticles ; i++)
		particles[i].next = &particles[i+1];
	particles[cl_numparticles-1].next = NULL;
}

void pSplashThink (cparticle_t *p, vec3_t org, vec3_t angle, float *alpha, float *size, int *image, float *time);

/*
===============
SWQ SPECIFIC STUFF...
===============
*/

void CL_SpeedTrail (vec3_t start, vec3_t end)
{
	cparticle_t *p;
	vec3_t		move;
	vec3_t		vec;
	float		len, dec, frac;

	VectorCopy (start, move);
	VectorSubtract (end, start, vec);
	len = VectorNormalize (vec);

	dec = 4;
	VectorScale (vec, dec, vec);

	frac = len/100;
	if (frac>1)
		frac=1;

	while (len > 0)
	{
		len -= dec;

		p = setupParticle (
			random()*360,	random()*15,	0,
			move[0] + crandom()*5.0,	move[1] + crandom()*5.0,	move[2] + crandom()*5.0,
			0,	0,	0,
			0,		0,		0,
			100,		175,		255,
			0,	0,	0,
			frac*0.5,		-1.0 / (0.8+frand()*0.2),
			15.0+random()*15, -5 + random()*2.5,			
			particle_smoke,
			PART_SHADED,
			pRotateThink,true);

		VectorAdd (move, vec, move);
	}
}

void pStunRotateThink (cparticle_t *p, vec3_t org, vec3_t angle, float *alpha, float *size, int *image, float *time)
{
	vec3_t angleVel;

	

	VectorSubtract(org, p->org, angleVel);
	VectorAdd(angle, angleVel, angle);
	VectorCopy(p->org, org);

	angle[YAW]		= sin(angle[YAW])*30;
	angle[PITCH]	= cos(angle[PITCH])*30;

	p->thinknext = true;
}

void CL_StunBlast (vec3_t pos, vec3_t color, float size)
{
	vec3_t dir, angles;
	int i;

	for (i=0;i<8;i++)
	{
		setupParticle (
			0,		0,		0,
			pos[0],		pos[1],		pos[2],
			crandom()*size,		crandom()*size,		crandom()*size,
			0,		0,		0,
			color[0],		color[1],		color[2],
			0,	0,	0,
			.5,		-1 / (0.8+frand()*0.2),
			size,	0,			
			particle_generic,
			0,
			0, false);
	}

	setupParticle (
		crandom()*360,		crandom()*360,		crandom()*360,
		pos[0],		pos[1],		pos[2],
		75,		300,	75,
		0,		0,		0,
		color[0],		color[1],		color[2],
		0,	0,	0,
		.75,		-1 / (0.8+frand()*0.2),
		size,	size*3,			
		particle_smoke,
		PART_ANGLED,
		pStunRotateThink, true);
}

void CL_LaserStun (vec3_t pos, vec3_t direction, vec3_t color, float size)
{
	vec3_t dir, angles;
	int i;

	for (i=0;i<16;i++)
	{
		vectoangles2(direction, angles);
		angles[PITCH]	+= crandom()*15;
		angles[YAW]		+= crandom()*15;
		AngleVectors(angles, dir, NULL, NULL);
		setupParticle (
			dir[0]*5,		dir[1]*5,		dir[2]*5,
			pos[0],		pos[1],		pos[2],
			dir[0]*10*size,	dir[1]*10*size,	dir[2]*10*size,
			0,		0,		0,
			color[0],		color[1],		color[2],
			0,	0,	0,
			.5,		-2.0 / (0.8+frand()*0.2),
			size/5.0,	0,			
			particle_generic,
			PART_DIRECTION,
			0, false);
	}
}

void CL_ForceTrail (vec3_t start, vec3_t end, qboolean light, float size)
{
	cparticle_t *p;
	vec3_t		move;
	vec3_t		vec;
	float		len, dec, length, frac;
	int			i=0;

	VectorCopy (start, move);
	VectorSubtract (end, start, vec);
	length = len = VectorNormalize (vec);

	dec = 1 + size/5;
	VectorScale (vec, dec, vec);

	while (len > 0)
	{
		len -= dec; i++;
		frac = len/length;

		if (light)
			p = setupParticle (
				random()*360,	random()*15,	0,
				move[0],	move[1],	move[2],
				0,	0,	0,
				0,		0,		0,
				150,		200,		255,
				0,	0,	0,
				1,		-2.0 / (0.8+frand()*0.2),
				size, size,			
				particle_smoke,
				0,
				pRotateThink,true);
		else
			p = setupParticle (
				random()*360,	random()*15,	0,
				move[0],	move[1],	move[2],
				0,	0,	0,
				0,		0,		0,
				255,		255,		255,
				0,	0,	0,
				1,		-2.0 / (0.8+frand()*0.2),
				size, size,			
				particle_inferno,
				0,
				pRotateThink,true);

		VectorAdd (move, vec, move);
	}
}

/*
===============
CL_FlameTrail -- DDAY SPECIFIC STUFF...
===============
*/

void CL_BlueFlameTrail (vec3_t start, vec3_t end)
{
	cparticle_t *p;
	vec3_t		move;
	vec3_t		vec;
	float		len, dec, length, frac;
	int			i=0;

	VectorCopy (start, move);
	VectorSubtract (end, start, vec);
	length = len = VectorNormalize (vec);

	dec = 1;
	VectorScale (vec, dec, vec);

	while (len > 0)
	{
		len -= dec; i++;
		frac = len/length;

		p = setupParticle (
			random()*360,	random()*15,	0,
			move[0],	move[1],	move[2],
			0,	0,	0,
			0,		0,		0,
			40 + 215*frac,		100,		255 - 215*frac,
			0,	0,	0,
			1,		-2.0 / (0.8+frand()*0.2),
			2.5 + 2.5*frac, 0,			
			particle_smoke,
			0,
			pRotateThink,true);

		VectorAdd (move, vec, move);
	}
}

void CL_InfernoTrail (vec3_t start, vec3_t end, float size)
{
	vec3_t		move;
	vec3_t		vec;
	float		len, dec, size2 = size * size;

	VectorCopy (start, move);
	VectorSubtract (end, start, vec);
	len = VectorNormalize (vec);

	dec = (20.0*size2+1);
	VectorScale (vec, dec, vec);

	while (len > 0)
	{
		len -= dec;

		setupParticle (
			random()*360,	random()*45,	0,
			move[0],	move[1],	move[2],
			0,	0,	0,
			0,		0,		0,
			255,		255,		255,
			0,	-100,	-200,
			1,		-1.0 / (0.8+frand()*0.2),
			1 + 20.0*size2, 5 + 50.0 * size2,	
			//1 + 50.0*size2*(random()*0.25 + .75), 0,			
			particle_inferno,
			0,
			pRotateThink,true);

		VectorAdd (move, vec, move);
	}
}


/*void CL_FlameBurst (vec3_t org, float size)
{
	int			i;

	for (i=0; i<16; i++)
		setupParticle (
			random()*360,	random()*180,	0,
			org[0] + crandom(),	org[1] + crandom(),	org[2] + crandom(),
			crandom() * size * 0.1,		crandom() * size * 0.1,		crandom() * size * 0.1,
			0,		0,		0,
			255,		255,		255,
			0,	-50,	-100,
			1,		-1.5 / (0.8+frand()*0.2),
			size, size,			
			particle_inferno,
			0,
			pRotateThink,true);
}*/

void CL_FlameTrail (vec3_t start, vec3_t end, float size)
{
	cparticle_t *p;
	vec3_t		move;
	vec3_t		vec;
	float		len, dec;
	int			i=0;

	VectorCopy (start, move);
	VectorSubtract (end, start, vec);
	len = VectorNormalize (vec);

	dec = (20.0*size+1);
	VectorScale (vec, dec, vec);

	while (len > 0)
	{
		len -= dec; i++;

		p = setupParticle (
			random()*360,	random()*15,	0,
			move[0] + crand()*3*size,	move[1] + crand()*3*size,	move[2] + crand()*3*size,
			crand()*size,	crand()*size,	crand()*size,
			0,		0,		0,
			255,		255,		255,
			0,	0,	0,
			1,		-2.0 / (0.8+frand()*0.2),
			5 + size*5, 175.0 * size,			
			particle_inferno,
			0,
			pRotateThink,true);

			if (p)
				addParticleLight (p,
					50+size*75, 0,
					0.5+random()*0.5, 0.5, 0.1);

		VectorAdd (move, vec, move);
	}

	if (size == 0)
	{
		VectorSubtract (start, end, vec);
		len = VectorNormalize (vec);
		VectorScale (vec, 25, vec);
		VectorAdd(vec, start, vec);

		CL_BlueFlameTrail(start, vec);
	}

}

void CL_Flame (vec3_t start)
{	int i;
	cparticle_t *p;


		p = setupParticle (
			random()*360,	random()*15,	0,
			start[0],	start[1],	start[2],
			crand()*10.0,	crand()*10.0,	random()*100.0,
			0,		0,		0,
			255,		255,		255,
			0,	0,	0,
			1,		-2.0 / (0.8+frand()*0.2),
			10, -10,			
			particle_inferno,
			0,
			pRotateThink,true);

			if (p)
				addParticleLight (p,
					20 + random()*20.0, 0,
					0.5+random()*0.5, 0.5, 0.1); //weak big
			if (p)
				addParticleLight (p,
					250.0, 0,
					0.01, 0.01, 0.01);
}

/*
===============
CL_BlasterTracer -- MOD SPECIFIC STUFF...
===============
*/

void CL_Tracer (vec3_t origin, vec3_t angle, int red, int green, int blue, float len, float size)
{
	vec3_t		dir;
	
	AngleVectors (angle, dir, NULL, NULL);
	VectorScale(dir, len,dir);

	setupParticle (
		dir[0],	dir[1],	dir[2],
		origin[0],	origin[1],	origin[2],
		0,	0,	0,
		0,		0,		0,
		red, green, blue,
		0,	0,	0,
		DIV254BY255,		INSTANT_PARTICLE,
		size,		0,			
		particle_generic,
		PART_DIRECTION|PART_TRANS,
		NULL,0);
}

void CL_Shield (vec3_t origin, float angle, int red, int green, int blue, float size, int times)
{
	int i;

	for (i=0; i<times; i++)
		setupParticle (
			0,	0,	angle,
			origin[0],	origin[1],	origin[2],
			0,	0,	0,
			0,		0,		0,
			red, green, blue,
			0,	0,	0,
			DIV254BY255,		INSTANT_PARTICLE,
			size,		0,			
			particle_shield,
			PART_DEPTHHACK_MID,
			NULL,0);
}

void CL_BlasterTracer (vec3_t origin, vec3_t angle, int red, int green, int blue, float len, float size)
{
	int i;
	vec3_t		dir;
	
	AngleVectors (angle, dir, NULL, NULL);
	VectorScale(dir, len,dir);

	for (i=0;i<3;i++)
		setupParticle (
			dir[0],	dir[1],	dir[2],
			origin[0],	origin[1],	origin[2],
			0,	0,	0,
			0,		0,		0,
			red, green, blue,
			0,	0,	0,
			DIV254BY255,		INSTANT_PARTICLE,
			size,		0,			
			particle_generic,
			PART_DIRECTION,
			NULL,0);
}

void CL_BlasterSplash (vec3_t origin, int red, int green, int blue, float size)
{
	int i;
	for (i=0;i<16;i++)
		setupParticle (
			origin[0],	origin[1],	origin[2],
			origin[0] + crandom()*size,	origin[1] + crandom()*size,	origin[2] + crandom()*size,
			0,	0,	0,
			0,		0,		0,
			red, green, blue,
			0,	0,	0,
			DIV254BY255,		INSTANT_PARTICLE,
			size*0.5f,		0,			
			particle_generic,
			PART_BEAM,
			NULL,0);
}

/*
===============
CL_LightningBeam
===============
*/

void CL_LightningBeam(vec3_t start, vec3_t end, int srcEnt, int dstEnt, float size)
{
	cparticle_t *list;
	cparticle_t *p=NULL;

	for (list=active_particles ; list ; list=list->next)
		if (list->src_ent == srcEnt && list->dst_ent == dstEnt && list->image == particle_lightning)
		{
			p=list;
			p->start = p->time = cl.time;
			VectorCopy(start, p->angle);
			VectorCopy(end, p->org);

			return;
		}

	p = setupParticle (
		start[0],	start[1],	start[2],
		end[0],		end[1],		end[2],
		0,	0,	0,
		0,		0,		0,
		255,	255,	255,
		0,	0,	0,
		1,		-2,
		size,		0,			
		particle_lightning,
		PART_LIGHTNING,
		0, false);

	if (!p)
		return;

	p->src_ent=srcEnt;
	p->dst_ent=dstEnt;
}

void CL_LightningFlare (vec3_t start, int srcEnt, int dstEnt)
{
	cparticle_t *list;
	cparticle_t *p=NULL;

	for (list=active_particles ; list ; list=list->next)
		if (list->src_ent == srcEnt && list->dst_ent == dstEnt && list->image == particle_lightflare)
		{
			p = list;
			p->start = p->time = cl.time;
			VectorCopy(start, p->org);
			return;
		}

	p = setupParticle (
		0,	0,	0,
		start[0],	start[1],	start[2],
		0,	0,	0,
		0,		0,		0,
		255,	255,	255,
		0,	0,	0,
		1,		-2.5,
		15,		0,			
		particle_lightflare,
		0,
		0, false);

	if (!p)
		return;

	p->src_ent=srcEnt;
	p->dst_ent=dstEnt;
}

/*
===============
CL_Explosion_Particle

BOOM!
===============
*/

void pExplosionThink (cparticle_t *p, vec3_t org, vec3_t angle, float *alpha, float *size, int *image, float *time)
{

	if (*alpha>.85)
		*image = particle_rexplosion1;
	else if (*alpha>.7)
		*image = particle_rexplosion2;
	else if (*alpha>.5)
		*image = particle_rexplosion3;
	else if (*alpha>.4)
		*image = particle_rexplosion4;
	else if (*alpha>.25)
		*image = particle_rexplosion5;
	else if (*alpha>.1)
		*image = particle_rexplosion6;
	else 
		*image = particle_rexplosion7;

	*alpha *= 3.0;

	if (*alpha > 1.0)
		*alpha = 1;

	p->thinknext = true;
}

#define EXPLODESTAININTESITY 75
void CL_Explosion_Particle (vec3_t org, float size, qboolean large, qboolean rocket)
{
	cparticle_t *p;

	if (large)
	{	
		if (size)
		{
			re.AddStain(org, size, -EXPLODESTAININTESITY,-EXPLODESTAININTESITY,-EXPLODESTAININTESITY);
		}
		else
		{
			if (rocket)
				re.AddStain(org, 45, -EXPLODESTAININTESITY,-EXPLODESTAININTESITY,-EXPLODESTAININTESITY);
			else
				re.AddStain(org, 65, -EXPLODESTAININTESITY,-EXPLODESTAININTESITY,-EXPLODESTAININTESITY);
		}

		p = setupParticle (
					0,		0,		0,
					org[0],	org[1],	org[2],
					0,		0,		0,
					0,		0,		0,
					255,	255,	255,
					0,		0,		0,
					1,		(0.5+random()*0.5) * (rocket)? -2 : -1.5,
					(size!=0)?size:(150-(!rocket)?75:0),	0,			
					particle_rexplosion1, //whatever :p
					PART_DEPTHHACK_SHORT,
					pExplosionThink, true);

		if (p)
		{	//smooth color blend :D
			float lightsize = (large)? 1.0 : 0.75;

			addParticleLight (p,
						lightsize*250, 0,
						1, 1, 1);
			addParticleLight (p,
						lightsize*265, 0,
						1, 0.75, 0);
			addParticleLight (p,
						lightsize*285, 0,
						1, 0.25, 0);
			addParticleLight (p,
						lightsize*300, 0,
						1, 0, 0);
		}
	}
/*	else //volumizers
	{
		setupParticle (
					0,		0,		0,
					org[0],	org[1],	org[2],
					0,		0,		0,
					0,		0,		0,
					255,	175,	100,
					0,		0,		0,
					1,		1 * (rocket)? -1.5 : -1.25,
					(size!=0)?size:(150-(!rocket)?75:0), 0,			
					particle_inferno,
					0,
					NULL,0);
	}*/
}


void pDisruptExplosionThink (cparticle_t *p, vec3_t org, vec3_t angle, float *alpha, float *size, int *image, float *time)
{

	if (*alpha>.66666)
		*image = particle_dexplosion1;
	else if (*alpha>.33333)
		*image = particle_dexplosion2;
	else
		*image = particle_dexplosion3;

	*alpha *= 3.0;

	if (*alpha > 1.0)
		*alpha = 1;

	p->thinknext = true;
}

void CL_Disruptor_Explosion_Particle (vec3_t org, float size)
{
	int i;
	float	alphastart = 1,
			alphadecel = -5;
	cparticle_t *p;

	re.AddStain(org, size, -EXPLODESTAININTESITY,-EXPLODESTAININTESITY,-EXPLODESTAININTESITY);

	//now add main sprite
	{
		p = setupParticle (
					0,		0,		0,
					org[0],	org[1],	org[2],
					0,		0,		0,
					0,		0,		0,
					255,	255,	255,
					0,		0,		0,
					alphastart,		alphadecel,
					size,	0,			
					particle_dexplosion1,
					PART_DEPTHHACK_SHORT,
					pDisruptExplosionThink, true);

		if (p)
		{
			float lightsize = size/150.0;

			addParticleLight (p,
						size*1.0f, 0,
						1, 1, 1);
			addParticleLight (p,
						size*1.25f, 0,
						0.75, 0, 1);
			addParticleLight (p,
						size*1.65f, 0,
						0.25, 0, 1);
			addParticleLight (p,
						size*1.9f, 0,
						0, 0, 1);
		}
	}
/*
	alphastart /= 10.0;
	alphadecel /= 10.0;

	//now make volumizers...
	for (i=0;i<256;i++)
	{
		int	size2 = size*2;	

		setupParticle (
					0,		0,		0,
					org[0]+ ((rand()%size2)-size)*0.1,	org[1]+ ((rand()%size2)-size)*0.1,	org[2]+ ((rand()%size2)-size)*0.1,
					((rand()%size2)-size)*2.0,	((rand()%size2)-size)*2.0,	((rand()%size2)-size)*2.0,
					0,		0,		0,
					100,	100,	200,
					0,		0,		0,
					alphastart,		alphadecel,
					size/3.0,	-10,			
					particle_smoke,
					PART_DIRECTION,
					pSplashThink,true);
	}*/
}

/*
===============
CL_WeatherFx

weather effects
===============
*/

//drop of blood
void CL_BloodSmack (vec3_t org);

void pRainSplashThink (cparticle_t *p, vec3_t org, vec3_t angle, float *alpha, float *size, int *image, float *time)
{
	//NO THINK!!!!
}

void pWeatherFXThink (cparticle_t *p, vec3_t org, vec3_t angle, float *alpha, float *size, int *image, float *time)
{
	float length;
	int i;
	vec3_t len, ang;

	VectorSubtract(p->angle, org, len);
	{
		float time1, time2;

		time1 = *time;
		time2 = time1*time1;

		for (i=0;i<2;i++)
			angle[i] = 0.2*(p->vel[i]*time1 + (p->accel[i])*time2);
		angle[2] = 0.2*(p->vel[2]*time1 + (p->accel[2]-PARTICLE_GRAVITY)*time2);

		length = VectorNormalize(angle);
		
		if (length>*size*10.0) 
			length = *size*10.0;
		
		VectorScale(angle, length, angle);
	}

	//now to trace for impact...
	{
		trace_t trace = CL_Trace (p->oldorg, org, 0.1, 1);
		if (trace.fraction < 1.0)//delete and stain...
		{
			switch ((int)p->temp)
			{
				case 0:	//RAIN

					//PARTICLE RECONSTRUCTION
					{
						vectoanglerolled(trace.plane.normal, rand()%360, p->angle);
						VectorCopy(trace.endpos, p->org);
						VectorClear(p->vel);
						VectorClear(p->accel);
						p->image = particle_smoke;
						p->flags = PART_SHADED|PART_ANGLED;
						p->alpha = *alpha;
						p->alphavel = -0.5;
						p->start = cl.time;
						p->think = pRainSplashThink;
						p->size = *size;
						p->sizevel = 10 + 10*random();
					}
					break;
				case 1:	//SNOW
				default:
					//kill this particle
					p->alpha = 0;
					
					*alpha=0;
					*size=0;
				break;
			}

		}
	}


	VectorCopy(org, p->oldorg);

	p->thinknext = true;
}

void CL_WeatherFx (vec3_t org, vec3_t vec, vec3_t color, int type, float size, float time)
{
	cparticle_t *p;
	int image, flags=0;

	switch (type)
	{
		case 0:	//RAIN
			image = particle_generic;
			flags = PART_SHADED|PART_DIRECTION|PART_GRAVITY;
			break;
		case 1:	//SNOW
			image = particle_generic;
			break;
		default:
			image = particle_generic;
			flags = PART_TRANS|PART_SHADED|PART_DIRECTION|PART_GRAVITY;
			break;
	}

	p = setupParticle (
		0 ,0 ,0,
		org[0], org[1], org[2],
		vec[0], vec[1], vec[2],
		0 ,0 ,0,
		color[0], color[1], color[2],
		0,			0,			0,
		1.0,		-1 / (1 + time),
		size,		0,			
		image,
		flags,
		pWeatherFXThink,true);

	if (p)
		p->temp = type;
}


/*
===============
CL_BloodHit

blood spray
===============
*/

#define MAXBLEEDSIZE 5

//drop of blood
void pBloodThink (cparticle_t *p, vec3_t org, vec3_t angle, float *alpha, float *size, int *image, float *time)
{
	float length;
	int i;
	vec3_t len;

	VectorSubtract(p->angle, org, len);
	{
		float time1, time2;

		time1 = *time;
		time2 = time1*time1;

		for (i=0;i<2;i++)
			angle[i] = 0.2*(p->vel[i]*time1 + (p->accel[i])*time2);
		angle[2] = 0.2*(p->vel[2]*time1 + (p->accel[2]-PARTICLE_GRAVITY)*time2);

		length = VectorNormalize(angle);
		if (length>MAXBLEEDSIZE) length = MAXBLEEDSIZE;
		VectorScale(angle, length, angle);
	}

	//now to trace for impact...
	{
		trace_t trace = CL_Trace (p->oldorg, org, length*0.5, 1);
		if (trace.fraction < 1.0)//delete and stain...
		{
			re.AddStain(org, 5+*alpha*10, 0, -50**alpha ,-50**alpha);

			*alpha=0;
			*size=0;
			p->alpha = 0;
		}
	}


	VectorCopy(org, p->oldorg);

	p->thinknext = true;
}

void CL_BloodSmack (vec3_t org)
{
	setupParticle (
			crand()*180, crand()*100, 0,
			org[0],	org[1],	org[2],
			0,	0,	0,
			0,			0,			0,
				255,		255,		255,
			0,			0,			0,
			1.0,		-0.5 / (0.5 + frand()*0.3),
			10,			0,			
			particle_redblood,
			PART_TRANS|PART_SHADED,
			pRotateThink,true);
}

void CL_BloodBleed (vec3_t org, vec3_t pos, vec3_t dir)
{
	setupParticle (
		org[0], org[1], org[2],
		org[0] + ((rand()&7)-4) + dir[0],	org[1] + ((rand()&7)-4) + dir[1],	org[2] + ((rand()&7)-4) + dir[2],
		pos[0]*(random()*3+5)*1.0,	pos[1]*(random()*3+5)*1.0,	pos[2]*(random()*3+5)*1.0,
		0,			0,			0,
		255,		255,		255,
		0,			0,			0,
		1.0,		-0.25 / (0.5 + frand()*0.3),
		MAXBLEEDSIZE*0.5f,		0,			
		particle_blooddrip,
		PART_TRANS|PART_SHADED|PART_DIRECTION|PART_GRAVITY,
		pBloodThink,true);
}

void CL_BloodHit (vec3_t org, vec3_t dir)
{
	vec3_t	move;
	int		i;

	VectorScale(dir, 3, move);

	switch ((int)(cl_blood->value))
	{
	case 1: //bleed
		for (i=0;i<3;i++)
			CL_BloodBleed (org, move, dir);
		break;
	case 2: //splat & bleed
	  CL_BloodSmack(org);
		for (i=0;i<5;i++)
			CL_BloodBleed (org, move, dir);
		break;
	case 3: //gore
		for (i=0;i<20;i++)
			CL_BloodBleed (org, move, dir);
		break;
	default: //splat
	  CL_BloodSmack(org);
		break;
	}

	
}

/*
===============
CL_ParticleEffect

Wall impact puffs
===============
*/
void CL_ParticleEffect (vec3_t org, vec3_t dir, int color8, int count)
{
	int			i;
	float		d;
	vec3_t color = { color8red(color8), color8green(color8), color8blue(color8)};

	for (i=0 ; i<count ; i++)
	{
		d = rand()&31;
		setupParticle (
			0,		0,		0,
			org[0] + ((rand()&7)-4) + d*dir[0],	org[1] + ((rand()&7)-4) + d*dir[1],	org[2] + ((rand()&7)-4) + d*dir[2],
			crand()*20,			crand()*20,			crand()*20,
			0,		0,		0,
			color[0],		color[1],		color[2],
			0,	0,	0,
			1.0,		-1.0 / (0.5 + frand()*0.3),
			1,			0,			
			particle_generic,
			PART_GRAVITY,
			NULL,0);
	}
}

/*
===============
CL_ParticleEffectSplash

Water Splashing
===============
*/
#define colorAdd 25
#define SplashSize 7.5
void pSplashThink (cparticle_t *p, vec3_t org, vec3_t angle, float *alpha, float *size, int *image, float *time)
{
	int i;
	vec3_t len;
	VectorSubtract(p->angle, org, len);
	
//	*size *= (float)(SplashSize/VectorLength(len)) * 0.5/((4-*size));
//	if (*size > SplashSize)
//		*size = SplashSize;

	//setting up angle for sparks
	{
		float time1, time2;

		time1 = *time;
		time2 = time1*time1;

		for (i=0;i<2;i++)
			angle[i] = 0.5*(p->vel[i]*time1 + (p->accel[i])*time2);
		angle[2] = 0.5*(p->vel[2]*time1 + (p->accel[2]-PARTICLE_GRAVITY)*time2);
	}

	p->thinknext = true;
}

void CL_ParticleEffectSplash (vec3_t org, vec3_t dir, int color8, int count)
{
	int			i;
	float		d;
	vec3_t color = { color8red(color8), color8green(color8), color8blue(color8)};

	for (i=0 ; i<count ; i++)
	{
		d = rand()&5;
		setupParticle (
			org[0],	org[1],	org[2],
			org[0]+d*dir[0],	org[1]+d*dir[1],	org[2]+d*dir[2],
			dir[0]*40 + crand()*10,	dir[1]*40 + crand()*10,	dir[2]*40 + crand()*10,
			0,		0,		0,
			color[0],	color[1],	color[2],
			0,	0,	0,
			1,		-1.0 / (0.5 + frand()*0.3),
			0.5,			0,			
			particle_generic,
			PART_GRAVITY|PART_DIRECTION   /*|PART_TRANS|PART_SHADED*/,
			pSplashThink,true);
	}
}


/*
===============
CL_ParticleEffect2
===============
*/
void CL_ParticleEffect2 (vec3_t org, vec3_t dir, int color8, int count)
{
	int			i;
	float		d;
	vec3_t color = { color8red(color8), color8green(color8), color8blue(color8)};

	for (i=0 ; i<count ; i++)
	{
		d = rand()&7;
		setupParticle (
			0,	0,	0,
			org[0]+((rand()&7)-4)+d*dir[0],	org[1]+((rand()&7)-4)+d*dir[1],	org[2]+((rand()&7)-4)+d*dir[2],
			crand()*20,			crand()*20,			crand()*20,
			0,		0,		0,
			color[0] + colorAdd,		color[1] + colorAdd,		color[2] + colorAdd,
			0,	0,	0,
			1,		-1.0 / (0.5 + frand()*0.3),
			1,			0,			
			particle_generic,
			PART_GRAVITY,
			NULL,0);
	}
}


// RAFAEL
/*
===============
CL_ParticleEffect3
===============
*/


void CL_ParticleEffect3 (vec3_t org, vec3_t dir, int color8, int count)
{
	int			i;
	float		d;
	vec3_t color = { color8red(color8), color8green(color8), color8blue(color8)};

	for (i=0 ; i<count ; i++)
	{
		d = rand()&7;
		setupParticle (
			0,	0,	0,
			org[0]+((rand()&7)-4)+d*dir[0],	org[1]+((rand()&7)-4)+d*dir[1],	org[2]+((rand()&7)-4)+d*dir[2],
			crand()*20,			crand()*20,			crand()*20,
			0,		0,		0,
			color[0] + colorAdd,		color[1] + colorAdd,		color[2] + colorAdd,
			0,	0,	0,
			1,		-1.0 / (0.5 + frand()*0.3),
			1,			0,			
			particle_generic,
			PART_GRAVITY,
			NULL, false);
	}
}

/*
===============
CL_ParticleEffectSparks
===============
*/

void pSparksThink (cparticle_t *p, vec3_t org, vec3_t angle, float *alpha, float *size, int *image, float *time)
{
	vec3_t dir;
	int i;

	//setting up angle for sparks
	{
		float time1, time2;

		time1 = *time;
		time2 = time1*time1;

		for (i=0;i<2;i++)
			angle[i] = 0.75*(p->vel[i]*time1 + (p->accel[i])*time2);
		angle[2] = 0.75*(p->vel[2]*time1 + (p->accel[2]-PARTICLE_GRAVITY)*time2);
	}

	p->thinknext = true;
}

void CL_ParticleEffectSparks (vec3_t org, vec3_t dir, vec3_t color, int count)
{
	int			i;
	float		d;
	for (i=0 ; i<count ; i++)
	{
		d = rand()&7;
		setupParticle (
			0,	0,	0,
			org[0]+((rand()&3)-2),	org[1]+((rand()&3)-2),	org[2]+((rand()&3)-2),
			//crandom()*20 + dir[0]*75,crand()*20 + dir[1]*75,
			//crand()*20 + dir[2]*75,
			20+dir[0]*75,20+dir[1]*75,20+dir[2]*75,
			0,		0,		0,
			color[0],		color[1],		color[2],
			0,	0,	0,
			0.75,		-1.5 / (0.5 + frand()*0.3),
			0.5,			0,			
			particle_generic,
			PART_GRAVITY|PART_DIRECTION,
			pSparksThink,true);
	}
}

/*
===============
CL_ParticleFootPrint
===============
*/

void CL_ParticleFootPrint (vec3_t org, vec3_t angle, float size, vec3_t color)
{
	float alpha = DIV254BY255;

	setupParticle (
		angle[0],	angle[1],	angle[2],
		org[0],	org[1],	org[2],
		0,		0,		0,
		0,		0,		0,
		color[0],		color[1],		color[2],
		0,		0,		0,
		alpha,			-.025,
		size,			0,			
		particle_footprint,
		PART_TRANS|PART_ANGLED|PART_SHADED,
		NULL, false);
}

/*
===============
CL_ParticleBulletDecal
===============
*/

void CL_ParticleBulletDecal (vec3_t org, vec3_t dir, float size)
{
	vec3_t		ang, angle;

	//return;

	VectorScale(dir, -1, angle);
	vectoanglerolled(angle, rand()%360, ang);

	setupParticle (
		ang[0],	ang[1],	ang[2],
		org[0]+dir[0],	org[1]+dir[1],	org[2]+dir[2],
		0,		0,		0,
		0,		0,		0,
		0,		0,		0,
		0,		0,		0,
		DIV254BY255,	-.1,
		size,			0,			
		particle_generic,
		PART_TRANS|PART_ANGLED|PART_SHADED,
		NULL, false);
}

/*
===============
CL_TeleporterParticles
===============
*/
void CL_TeleporterParticles (entity_state_t *ent)
{
	int			i;

	for (i=0 ; i<8 ; i++)
	{
		setupParticle (
			0,	0,	0,
			ent->origin[0]-16+(rand()&31),	ent->origin[1]-16+(rand()&31),	ent->origin[2]-16+(rand()&31),
			crand()*14,			crand()*14,			crand()*14,
			0,		0,		0,
			200+rand()*50,		200+rand()*50,		200+rand()*50,
			0,	0,	0,
			1,		-0.5,
			1,			0,			
			particle_generic,
			PART_GRAVITY,
			NULL,0);
	}
}


/*
===============
CL_LogoutEffect

===============
*/
void CL_LogoutEffect (vec3_t org, int type)
{
	int			i;
	vec3_t	color;

	for (i=0 ; i<500 ; i++)
	{
		if (type == MZ_LOGIN)// green
		{
			color[0] = 20;
			color[1] = 200;
			color[2] = 20;
		}
		else if (type == MZ_LOGOUT)// red
		{
			color[0] = 200;
			color[1] = 20;
			color[2] = 20;
		}
		else// yellow
		{
			color[0] = 200;
			color[1] = 200;
			color[2] = 20;
		}
		
		setupParticle (
			0,	0,	0,
			org[0] - 16 + frand()*32,	org[1] - 16 + frand()*32,	org[2] - 24 + frand()*56,
			crand()*20,			crand()*20,			crand()*20,
			0,		0,		0,
			color[0],		color[1],		color[2],
			0,	0,	0,
			1,		-1.0 / (1.0 + frand()*0.3),
			1,			0,			
			particle_generic,
			PART_GRAVITY,
			NULL,0);
	}
}


/*
===============
CL_ItemRespawnParticles

===============
*/
void CL_ItemRespawnParticles (vec3_t org)
{
	int			i;

	for (i=0 ; i<64 ; i++)
	{
		setupParticle (
			0,	0,	0,
			org[0] + crand()*8,	org[1] + crand()*8,	org[2] + crand()*8,
			crand()*8,			crand()*8,			crand()*8,
			0,		0,		PARTICLE_GRAVITY*0.2,
			0,		150+rand()*25,		0,
			0,	0,	0,
			1,		-1.0 / (1.0 + frand()*0.3),
			1,			0,			
			particle_generic,
			PART_GRAVITY,
			NULL,0);
	}
}


/*
===============
CL_ExplosionParticles
===============
*/

void pExplosionSparksThink (cparticle_t *p, vec3_t org, vec3_t angle, float *alpha, float *size, int *image, float *time)
{
	int i;

	//setting up angle for sparks
	{
		float time1, time2;

		time1 = *time;
		time2 = time1*time1;

		for (i=0;i<2;i++)
			angle[i] = 0.25*(p->vel[i]*time1 + (p->accel[i])*time2);
		angle[2] = 0.25*(p->vel[2]*time1 + (p->accel[2]-PARTICLE_GRAVITY)*time2);

	}

	p->thinknext = true;
}

void CL_ExplosionParticles (vec3_t org, float scale)
{
	vec3_t vel;
	int			i;

	for (i=0 ; i<256 ; i++)
	{
		VectorSet(vel, crandom(), crandom(), crandom());
		VectorNormalize(vel);
		VectorScale(vel, scale*75.0f, vel);

		setupParticle (
			0,	0,	0,
			org[0] + ((rand()%32)-16),	org[1] + ((rand()%32)-16),	org[2] + ((rand()%32)-16),
			vel[0], vel[1], vel[2],
			0,		0,		0,
			255,	255,	255,
			0,		-200,	-400,
			1,		-0.8 / (0.5 + frand()*0.3),
			scale,		-scale,			
			particle_blaster,
			PART_GRAVITY|PART_DIRECTION,//PART_SPARK,
			pExplosionSparksThink, true);
	}
}

/*
===============
CL_BigTeleportParticles
===============
*/
void CL_BigTeleportParticles (vec3_t org)
{
	int			i, index;
	float		angle, dist;
	static int colortable0[4] = {10,50,150,50};
	static int colortable1[4] = {150,150,50,10};
	static int colortable2[4] = {50,10,10,150};

	for (i=0 ; i<4096 ; i++)
	{

		index = rand()&3;
		angle = M_PI*2*(rand()&1023)/1023.0;
		dist = rand()&31;
		setupParticle (
			0,	0,	0,
			org[0]+cos(angle)*dist,	org[1] + sin(angle)*dist,org[2] + 8 + (rand()%90),
			cos(angle)*(70+(rand()&63)),sin(angle)*(70+(rand()&63)),-100 + (rand()&31),
			-cos(angle)*100,	-sin(angle)*100,PARTICLE_GRAVITY*4,
			colortable0[index],	colortable1[index],	colortable2[index],
			0,	0,	0,
			1,		-0.3 / (0.5 + frand()*0.3),
			1,		0.3 / (0.5 + frand()*0.3),			
			particle_generic,
			0,
			NULL,0);
	}
}


/*
===============
CL_BlasterParticles

Wall impact puffs
===============
*/
#define pBlasterMaxSize 5
void pBlasterThink (cparticle_t *p, vec3_t org, vec3_t angle, float *alpha, float *size, int *image, float *time)
{
	int i;
	vec3_t len;
	VectorSubtract(p->angle, org, len);
	
	*size *= (float)(pBlasterMaxSize/VectorLength(len)) * 1.0/((4-*size));
	if (*size > pBlasterMaxSize)
		*size = pBlasterMaxSize;
	
	p->thinknext = true;
}

void CL_BlasterParticles (vec3_t org, vec3_t dir, int count)
{
	int			i;
	float		d;
	float speed = .75;

	for (i=0 ; i<count ; i++)
	{
		d = rand()&5;
		setupParticle (
			org[0],	org[1],	org[2],
			org[0]+((rand()&5)-2)+d*dir[0],	org[1]+((rand()&5)-2)+d*dir[1],	org[2]+((rand()&5)-2)+d*dir[2],
			(dir[0]*75 + crand()*20)*speed,	(dir[1]*75 + crand()*20)*speed,	(dir[2]*75 + crand()*20)*speed,
			0,		0,		0,
			255,		150,		50,
			0,	-90,	-30,
			1,		-1.0 / (0.5 + frand()*0.3),
			4,			-1,			
			particle_generic,
			PART_GRAVITY,
			pBlasterThink,true);
	}
}

/*
===============
CL_BlasterTrail

===============
*/
void CL_BlasterTrail (vec3_t start, vec3_t end)
{
	vec3_t		move;
	vec3_t		vec;
	float		len;
	int			dec;

	VectorCopy (start, move);
	VectorSubtract (end, start, vec);
	len = VectorNormalize (vec);

	dec = 4;
	VectorScale (vec, dec, vec);

	// FIXME: this is a really silly way to have a loop
	while (len > 0)
	{
		len -= dec;

		setupParticle (
			0,	0,	0,
			move[0] + crand(),	move[1] + crand(),	move[2] + crand(),
			crand()*5,	crand()*5,	crand()*5,
			0,		0,		0,
			255,		150,		50,
			0,	-90,	-30,
			1,		-1.0 / (0.5 + frand()*0.3),
			4,			-6,			
			particle_generic,
			0,
			NULL,0);

		VectorAdd (move, vec, move);
	}
}

/*
===============
CL_QuadTrail

===============
*/
void CL_QuadTrail (vec3_t start, vec3_t end)
{
	vec3_t		move;
	vec3_t		vec;
	float		len;
	int			dec;

	VectorCopy (start, move);
	VectorSubtract (end, start, vec);
	len = VectorNormalize (vec);

	dec = 5;
	VectorScale (vec, 5, vec);

	while (len > 0)
	{
		len -= dec;

		setupParticle (
			0,	0,	0,
			move[0] + crand()*16,	move[1] + crand()*16,	move[2] + crand()*16,
			crand()*5,	crand()*5,	crand()*5,
			0,		0,		0,
			0,		0,		200,
			0,	0,	0,
			1,		-1.0 / (0.8+frand()*0.2),
			1,			0,			
			particle_generic,
			0,
			NULL,0);

		VectorAdd (move, vec, move);
	}
}

/*
===============
CL_FlagTrail

===============
*/
void CL_FlagTrail (vec3_t start, vec3_t end, qboolean isred)
{
	vec3_t		move;
	vec3_t		vec;
	float		len;
	int			dec;

	VectorCopy (start, move);
	VectorSubtract (end, start, vec);
	len = VectorNormalize (vec);

	dec = 5;
	VectorScale (vec, 5, vec);

	while (len > 0)
	{
		len -= dec;

		setupParticle (
			0,	0,	0,
			move[0] + crand()*16,	move[1] + crand()*16,	move[2] + crand()*16,
			crand()*5,	crand()*5,	crand()*5,
			0,		0,		0,
			(isred)?255:0,		0,		(!isred)?255:0,
			0,	0,	0,
			1,		-1.0 / (0.8+frand()*0.2),
			1,			0,			
			particle_generic,
			0,
			NULL,0);

		VectorAdd (move, vec, move);
	}
}

/*
===============
CL_DiminishingTrail

===============
*/
void CL_DiminishingTrail (vec3_t start, vec3_t end, centity_t *old, int flags)
{
	vec3_t		move;
	vec3_t		vec;
	float		len;
	float		dec;
	float		orgscale;
	float		velscale;

	VectorCopy (start, move);
	VectorSubtract (end, start, vec);
	len = VectorNormalize (vec);

	dec = (flags & EF_ROCKET) ? 10 : 2;
	VectorScale (vec, dec, vec);

	if (old->trailcount > 900)
	{
		orgscale = 4;
		velscale = 15;
	}
	else if (old->trailcount > 800)
	{
		orgscale = 2;
		velscale = 10;
	}
	else
	{
		orgscale = 1;
		velscale = 5;
	}

	while (len > 0)
	{
		len -= dec;

		if (!free_particles)
			return;

		if (flags & EF_ROCKET)
		{
			setupParticle (
				crand()*180, crand()*100, 0,
				move[0],	move[1],	move[2],
				crand()*5,	crand()*5,	crand()*5,
				0,		0,		5,
				255,	255,	255,
				-50,	-50,	-50,
				0.75,		-0.5,
				5,			12.5,			
				particle_smoke,
				PART_TRANS|PART_SHADED,
				pRotateThink, true);
		}
		else
		{
			// drop less particles as it flies
			if ((rand()&1023) < old->trailcount)
			{
			  if (flags & EF_GIB)
				{
				  setupParticle (
						0,	0,	random()*360,
						move[0] + crand()*orgscale,	move[1] + crand()*orgscale,	move[2] + crand()*orgscale,
						crand()*velscale,	crand()*velscale,	crand()*velscale,
						0,		0,		0,
						255,		255,		255,
						0,	0,	0,
						0.75,		-0.75 / (1+frand()*0.4),
						1+3*frand(),			-1,			
						particle_blooddrop,
						PART_TRANS|PART_GRAVITY|PART_SHADED,
						NULL,0);
				}
				else if (flags & EF_GREENGIB)
				{
					setupParticle (
						0,	0,	0,
						move[0] + crand()*orgscale,	move[1] + crand()*orgscale,	move[2] + crand()*orgscale,
						crand()*velscale,	crand()*velscale,	crand()*velscale,
						0,		0,		0,
						0,		255,		0,
						0,	0,	0,
						0,		-1.0 / (1+frand()*0.4),
						5,			-1,			
						particle_blood,
						PART_GRAVITY|PART_SHADED,
						NULL,0);
				}
				else
				{
					setupParticle (
						crand()*180, crand()*50, 0,
						move[0] + crand()*orgscale,	move[1] + crand()*orgscale,	move[2] + crand()*orgscale,
						crand()*velscale,	crand()*velscale,	crand()*velscale,
						0,		0,		20,
						255,		255,		255,
						0,	0,	0,
						0.5,		-0.5,
						5,			5,			
						particle_smoke,
						PART_TRANS|PART_SHADED,
						pRotateThink,true);
				}
			}

			old->trailcount -= 5;
			if (old->trailcount < 100)
				old->trailcount = 100;
		}

		VectorAdd (move, vec, move);
	}
}

void MakeNormalVectors (vec3_t forward, vec3_t right, vec3_t up)
{
	float		d;

	// this rotate and negat guarantees a vector
	// not colinear with the original
	right[1] = -forward[0];
	right[2] = forward[1];
	right[0] = forward[2];

	d = DotProduct (right, forward);
	VectorMA (right, -d, forward, right);
	VectorNormalize (right);
	CrossProduct (right, forward, up);
}

/*
===============
CL_RocketTrail

===============
*/
void CL_RocketTrail (vec3_t start, vec3_t end, centity_t *old)
{
	vec3_t		move;
	vec3_t		vec;
	float		len, totallen;
	float		dec;

	// smoke
	CL_DiminishingTrail (start, end, old, EF_ROCKET);

	// fire
	VectorCopy (start, move);
	VectorSubtract (end, start, vec);
	totallen = len = VectorNormalize (vec);

	dec = 1;
	VectorScale (vec, dec, vec);

	while (len > 0)
	{
		len -= dec;

		if (!free_particles)
			return;
/*
		//flame from rocket
		setupParticle (
			0,	0,	0,
			move[0],move[1],move[2],
			vec[0],	vec[1],	vec[2],
			0,		0,		0,
			255,		200,		100,
			0,	0,	-200,
			1,			-15,
			2.0*(2-len/totallen), -15,			
			particle_generic,
			0,
			NULL,0);
//*/
//*
		//falling particles
		if ( (rand()&7) == 0)
		{
			setupParticle (
				0,	0,	0,
				move[0] + crand()*5,	move[1] + crand()*5,	move[2] + crand()*5,
				crand()*20,	crand()*20,	crand()*20,
				0,		0,		20,
				255,	255,	255,
				0,		-100,	-200,
				1,		-2.0 / (1+frand()*0.2),
				1,			-3,			
				particle_blaster,
				PART_GRAVITY,
				NULL,0);
		}
//*/
		VectorAdd (move, vec, move);
	}
}

/*
===============
CL_RailTrail

===============
*/
#define RAILSPACE 1.0

void CL_RailSprial (vec3_t start, vec3_t end)
{
	vec3_t		move;
	vec3_t		vec;
	float		len;
	vec3_t		right, up;
	int			i;
	float		d, c, s;
	vec3_t		dir;

	VectorCopy (start, move);
	VectorSubtract (end, start, vec);
	len = VectorNormalize (vec);

	MakeNormalVectors (vec, right, up);

	VectorScale(vec, RAILSPACE, vec);


	for (i=0 ; i<len ; i+=RAILSPACE)
	{
		d = i * 0.1;
		c = cos(d);
		s = sin(d);

		VectorScale (right, c, dir);
		VectorMA (dir, s, up, dir);

		setupParticle (
			0,	0,	0,
			move[0] + dir[0]*3,	move[1] + dir[1]*3,	move[2] + dir[2]*3,
			dir[0]*6,	dir[1]*6,	dir[2]*6,
			0,		0,		0,
			cl_railred->value,	cl_railgreen->value,	cl_railblue->value,
			0,	0,	0,
			1,		-1.0,
			3*RAILSPACE,	0,			
			particle_generic,
			0,
			NULL,0);

		VectorAdd (move, vec, move);
	}
}

void pDevRailThink (cparticle_t *p, vec3_t org, vec3_t angle, float *alpha, float *size, int *image, float *time)
{
	int i;
	vec3_t len;
	VectorSubtract(p->angle, org, len);
	
	*size *= (float)(SplashSize/VectorLength(len)) * 0.5/((4-*size));
	if (*size > SplashSize)
		*size = SplashSize;

	//setting up angle for sparks
	{
		float time1, time2;

		time1 = *time;
		time2 = time1*time1;

		for (i=0;i<2;i++)
			angle[i] = 3*(p->vel[i]*time1 + (p->accel[i])*time2);
		angle[2] = 3*(p->vel[2]*time1 + (p->accel[2]-PARTICLE_GRAVITY)*time2);
	}

	p->thinknext = true;
}
#define DEVRAILSTEPS 2
void CL_DevRailTrail (vec3_t start, vec3_t end)
{
	vec3_t		move, last;
	vec3_t		vec, point;
	float		len;
	int			dec, i=0;

	VectorCopy (start, move);
	VectorSubtract (end, start, vec);
	len = VectorNormalize (vec);
	VectorCopy(vec, point);

	dec = 4;
	VectorScale (vec, dec, vec);

	// FIXME: this is a really silly way to have a loop
	while (len > 0)
	{
		len -= dec;
		i++;

		VectorCopy (move, last);
		VectorAdd (move, vec, move);

		if (i>=DEVRAILSTEPS)
		{
			for (i=3;i>0;i--)
			setupParticle (
				last[0],	last[1],	last[2],
				move[0],	move[1],	move[2],
				0,		0,		0,
				0,		0,		0,
				cl_railred->value,		cl_railgreen->value,		cl_railblue->value,
				0,	-90,	-30,
				1,		-.75,
				dec*DEVRAILSTEPS*TWOTHIRDS,	0,			
				particle_beam,
				PART_BEAM,
				NULL,0);
		}

			setupParticle (
				0,	0,	0,
				move[0],	move[1],	move[2],
				crand()*10,	crand()*10,	crand()*10+20,
				0,		0,		0,
				cl_railred->value,	cl_railgreen->value,	cl_railblue->value,
				0,	0,	0,
				1,		-0.75 / (0.5 + frand()*0.3),
				2,			-0.25,			
				0,
				PART_GRAVITY|PART_DIRECTION,
				pDevRailThink,true);

			setupParticle (
				crand()*180, crand()*100, 0,
				move[0],	move[1],	move[2],
				crand()*10,	crand()*10,	crand()*10+20,
				0,		0,		5,
				255,	255,	255,
				0,	0,	0,
				0.25,		-0.25,
				5,			10,			
				particle_smoke,
				PART_TRANS|PART_GRAVITY,
				pRotateThink, true);
	}
}

#define RAILTRAILSPACE 15
void CL_RailTrail (vec3_t start, vec3_t end)
{
	vec3_t		move, last;
	vec3_t		vec, point;
	int			i;
	float		len;
	float		dec;
	vec3_t		right, up;
	qboolean	colored = (cl_railtype->value!=1);

	VectorCopy (start, move);
	VectorSubtract (end, start, vec);
	len = VectorNormalize (vec);
	VectorCopy (vec, point);

	MakeNormalVectors (vec, right, up);
	VectorScale (vec, (cl_railtype->value == 2)?RAILTRAILSPACE:RAILSPACE, vec);
	VectorCopy (start, move);

	
	if (cl_railtype->value == 3)
	{
		CL_DevRailTrail(start, end);
		return;
	}

	while (len > 0)
	{	
		VectorCopy (move, last);	
		VectorAdd (move, vec, move);

		if (cl_railtype->value == 2)
		{
			len -= RAILTRAILSPACE;

			for (i=0;i<3;i++)
				setupParticle (
					last[0],	last[1],	last[2],
					move[0],	move[1],	move[2],
					0,	0,	0,
					0,		0,		0,
					cl_railred->value,	cl_railgreen->value,	cl_railblue->value,
					0,	0,	0,
					1,		-1.0,
					RAILTRAILSPACE*TWOTHIRDS,	0,			
					particle_beam,
					PART_BEAM,
					NULL,0);

		}
		else
		{
			len -= RAILSPACE;

			setupParticle (
			0,	0,	0,
			move[0],	move[1],	move[2],
			0,	0,	0,
			0,		0,		0,
			(colored)?cl_railred->value:255,	(colored)?cl_railgreen->value:255,	(colored)?cl_railblue->value:255,
			0,	0,	0,
			1,		-1.0,
			3*RAILSPACE,	0,			
			particle_generic,
			0,
			NULL,0);
		}
	}

	if (cl_railtype->value == 1)
		CL_RailSprial (start, end);
}

// RAFAEL
/*
===============
CL_IonripperTrail
===============
*/
void CL_IonripperTrail (vec3_t ent, vec3_t start)
{
	vec3_t	move, last;
	vec3_t	vec, aim;
	float	len;
	float	dec;
	float	overlap;

	VectorCopy (start, move);
	VectorSubtract (ent, start, vec);
	len = VectorNormalize (vec);
	VectorCopy(vec, aim);

	dec = len*0.2;
	if (dec<1)
		dec=1;

	overlap = dec*5.0;

	VectorScale (vec, dec, vec);

	while (len > 0)
	{
		len -= dec;

		VectorCopy(move, last);
		VectorAdd (move, vec, move);

		setupParticle (
			last[0],	last[1],	last[2],
			move[0]+aim[0]*overlap,	move[1]+aim[1]*overlap,	move[2]+aim[2]*overlap,
			0,	0,	0,
			0,		0,		0,
			255,	100,	0,
			0,	0,	0,
			0.5,	-1.0 / (0.3 + frand() * 0.2),
			3,			3,			
			particle_generic,
			PART_BEAM,
			NULL,0);
	}

	setupParticle (
	0,	0,	0,
	start[0],	start[1],	start[2],
	0,	0,	0,
	0,		0,		0,
	255,	100,	0,
	0,	0,	0,
	0.5,	-1.0 / (0.3 + frand() * 0.2),
	3,			3,			
	particle_generic,
	0,
	NULL,0);
}


/*
===============
CL_BubbleTrail

===============
*/
void CL_BubbleTrail (vec3_t start, vec3_t end)
{
	vec3_t		move;
	vec3_t		vec;
	float		len;
	int			i;
	float		dec;

	VectorCopy (start, move);
	VectorSubtract (end, start, vec);
	len = VectorNormalize (vec);

	dec = 32;
	VectorScale (vec, dec, vec);

	for (i=0 ; i<len ; i+=dec)
	{

		setupParticle (
			0,	0,	0,
			move[0]+crand()*2,	move[1]+crand()*2,	move[2]+crand()*2,
			crand()*5,	crand()*5,	crand()*5+6,
			0,		0,		0,
			255,	255,	255,
			0,	0,	0,
			0.75,		-1.0 / (1 + frand() * 0.2),
			(frand()>0.25)? 1 : (frand()>0.5) ? 2 : (frand()>0.75) ? 3 : 4,			1,			
			particle_bubble,
			PART_TRANS|PART_SHADED,
			NULL,0);

		VectorAdd (move, vec, move);
	}
}


/*
===============
CL_FlyParticles
===============
*/

#define	BEAMLENGTH			16
void CL_FlyParticles (vec3_t origin, int count)
{
	int			i;
	float		angle;
	float		sr, sp, sy, cr, cp, cy;
	vec3_t		forward;
	float		dist = 64;
	float		ltime;


	if (count > NUMVERTEXNORMALS)
		count = NUMVERTEXNORMALS;

	if (!avelocities[0][0])
	{
		for (i=0 ; i<NUMVERTEXNORMALS*3 ; i++)
			avelocities[0][i] = (rand()&255) * 0.01;
	}


	ltime = (float)cl.time / 1000.0;
	for (i=0 ; i<count ; i+=2)
	{
		angle = ltime * avelocities[i][0];
		sy = sin(angle);
		cy = cos(angle);
		angle = ltime * avelocities[i][1];
		sp = sin(angle);
		cp = cos(angle);
		angle = ltime * avelocities[i][2];
		sr = sin(angle);
		cr = cos(angle);
	
		forward[0] = cp*cy;
		forward[1] = cp*sy;
		forward[2] = -sp;

		dist = sin(ltime + i)*64;

		setupParticle (
			0,	0,	0,
			origin[0] + bytedirs[i][0]*dist + forward[0]*BEAMLENGTH,origin[1] + bytedirs[i][1]*dist + forward[1]*BEAMLENGTH,
			origin[2] + bytedirs[i][2]*dist + forward[2]*BEAMLENGTH,
			0,	0,	0,
			0,	0,	0,
			0,	0,	0,
			0,	0,	0,
			1,		-100,
			1+sin(i+ltime),	1,			
			particle_generic,
			PART_TRANS,
			NULL,0);
	}
}

void CL_FlyEffect (centity_t *ent, vec3_t origin)
{
	int		n;
	int		count;
	int		starttime;

	if (ent->fly_stoptime < cl.time)
	{
		starttime = cl.time;
		ent->fly_stoptime = cl.time + 60000;
	}
	else
	{
		starttime = ent->fly_stoptime - 60000;
	}

	n = cl.time - starttime;
	if (n < 20000)
		count = n * 162 / 20000.0;
	else
	{
		n = ent->fly_stoptime - cl.time;
		if (n < 20000)
			count = n * 162 / 20000.0;
		else
			count = 162;
	}

	CL_FlyParticles (origin, count);
}


/*
===============
CL_BfgParticles
===============
*/
void pBFGThink (cparticle_t *p, vec3_t org, vec3_t angle, float *alpha, float *size, int *image, float *time)
{
	vec3_t len;
	VectorSubtract(p->angle, p->org, len);
	
	*size = (float)((300/VectorLength(len))*0.75);
}

#define	BEAMLENGTH			16
void CL_BfgParticles (entity_t *ent)
{
	int			i;
	cparticle_t	*p;
	float		angle;
	float		sr, sp, sy, cr, cp, cy;
	vec3_t		forward;
	float		dist = 64, dist2;
	vec3_t		v;
	float		ltime;
	
	if (!avelocities[0][0])
	{
		for (i=0 ; i<NUMVERTEXNORMALS*3 ; i++)
			avelocities[0][i] = (rand()&255) * 0.01;
	}


	ltime = (float)cl.time / 1000.0;
	for (i=0 ; i<NUMVERTEXNORMALS ; i++)
	{
		angle = ltime * avelocities[i][0];
		sy = sin(angle);
		cy = cos(angle);
		angle = ltime * avelocities[i][1];
		sp = sin(angle);
		cp = cos(angle);
		angle = ltime * avelocities[i][2];
		sr = sin(angle);
		cr = cos(angle);
	
		forward[0] = cp*cy;
		forward[1] = cp*sy;
		forward[2] = -sp;

		dist2 = dist;
		dist = sin(ltime + i)*64;

		p = setupParticle (
			ent->origin[0],	ent->origin[1],	ent->origin[2],
			ent->origin[0] + bytedirs[i][0]*dist + forward[0]*BEAMLENGTH,ent->origin[1] + bytedirs[i][1]*dist + forward[1]*BEAMLENGTH,
			ent->origin[2] + bytedirs[i][2]*dist + forward[2]*BEAMLENGTH,
			0,	0,	0,
			0,	0,	0,
			50,	200*dist2,	20,
			0,	0,	0,
			1,		-100,
			1,			1,			
			particle_generic,
			0,
			pBFGThink, true);
		
		if (!p)
			return;

		VectorSubtract (p->org, ent->origin, v);
		dist = VectorLength(v) / 90.0;
	}
}


/*
===============
CL_TrapParticles
===============
*/
// RAFAEL
void CL_TrapParticles (entity_t *ent)
{
	int colors[][3] =
	{
		{255, 200, 150},
		{255, 200, 100},
		{255, 200, 50},
		{0, 0, 0}
	};
	vec3_t		move;
	vec3_t		vec;
	vec3_t		start, end;
	float		len;
	int			dec, index;

	ent->origin[2]-=14;
	VectorCopy (ent->origin, start);
	VectorCopy (ent->origin, end);
	end[2]+=64;

	VectorCopy (start, move);
	VectorSubtract (end, start, vec);
	len = VectorNormalize (vec);

	dec = 2.5;
	VectorScale (vec, dec, vec);

	// FIXME: this is a really silly way to have a loop
	while (len > 0)
	{
		len -= dec;

		index = rand()&3;

		setupParticle (
			0,	0,	0,
			move[0] + crand(),	move[1] + crand(),	move[2] + crand(),
			crand()*15,	crand()*15,	crand()*15,
			0,	0,	PARTICLE_GRAVITY,
			colors[index][0],	colors[index][1],	colors[index][2],
			0,	0,	0,
			1,		-1.0 / (0.3+frand()*0.2),
			3,			-5,			
			particle_generic,
			0,
			NULL,0);

		VectorAdd (move, vec, move);
	}

	{

		int			i, j, k;
		float		vel;
		vec3_t		dir;
		vec3_t		org;

		
		ent->origin[2]+=14;
		VectorCopy (ent->origin, org);


		for (i=-2 ; i<=2 ; i+=4)
			for (j=-2 ; j<=2 ; j+=4)
				for (k=-2 ; k<=4 ; k+=4)
				{

					dir[0] = j * 8;
					dir[1] = i * 8;
					dir[2] = k * 8;
		
					VectorNormalize (dir);						
					vel = 50 + (rand()&63);

					index = rand()&3;

					setupParticle (
						0,	0,	0,
						org[0] + i + ((rand()&23) * crand()), org[1] + j + ((rand()&23) * crand()),	org[2] + k + ((rand()&23) * crand()),
						dir[0]*vel,	dir[1]*vel,	dir[2]*vel,
						0,	0,	0,
						colors[index][0],	colors[index][1],	colors[index][2],
						0,	0,	0,
						1,		-1.0 / (0.3+frand()*0.2),
						3,			-10,			
						particle_generic,
						PART_GRAVITY,
						NULL,0);
				}
	}
}


/*
===============
CL_BFGExplosionParticles
===============
*/
//FIXME combined with CL_ExplosionParticles
void CL_BFGExplosionParticles (vec3_t org)
{
	int			i;

	for (i=0 ; i<256 ; i++)
	{
		setupParticle (
			0,	0,	0,
			org[0] + ((rand()%32)-16), org[1] + ((rand()%32)-16),	org[2] + ((rand()%32)-16),
			(rand()%150)-75,	(rand()%150)-75,	(rand()%150)-75,
			0,	0,	0,
			200,	100+rand()*50,	0,
			0,	0,	0,
			1,		-0.8 / (0.5 + frand()*0.3),
			10,			-10,			
			particle_generic,
			PART_GRAVITY,
			NULL,0);
	}
}


/*
===============
CL_TeleportParticles

===============
*/

void CL_MakeTeleportParticles (vec3_t org, float min, float max, float size, 
							   int red, int green, int blue, particle_type particleType)
{
	cparticle_t		*p;
	int			i, j, k;
	float		vel, resize = 1;
	vec3_t		dir, temp;

	resize += size/10;

	for (i=-16 ; i<=16 ; i+=4)
		for (j=-16 ; j<=16 ; j+=4)
			for (k=min ; k<=max ; k+=4)
			{
				dir[0] = j*8;
				dir[1] = i*8;
				dir[2] = k*8;

				VectorNormalize (dir);						
				vel = (rand()&63);

				p = setupParticle (
					0,	0,	0,
					org[0]+ (i+(rand()&3))*resize, org[1]+(j+(rand()&3))*resize,	org[2]+(k+(rand()&3))*resize,
					dir[0]*vel,	dir[1]*vel,	dir[2]*(25 + vel),
					0,	0,	0,
					red, green, blue,
					0,	0,	0,
					1,		-0.75 / (0.3 + (rand()&7) * 0.02),
					(random()*.25+.75)*size*resize,			0,			
					particleType,
					PART_GRAVITY,
					NULL,0);

				if (!p)
					continue;

				VectorCopy(p->org, temp); temp[2] = org[2];
				VectorSubtract(org, temp, p->vel);
				p->vel[2]+=25;
				
				VectorScale(p->vel, 3, p->accel);
			}
}

void CL_TeleportParticles (vec3_t org)
{
	CL_MakeTeleportParticles (org, -16, 32, 
		2.5, 255,255,200, particle_generic);
}

void CL_Disintegrate (vec3_t pos, int ent)
{
	CL_MakeTeleportParticles (pos, -16, 24, 
		7.5, 100, 100, 255, particle_smoke);
}

void CL_FlameBurst (vec3_t pos, float size)
{
	CL_MakeTeleportParticles (pos, -16, 24, 
		size, 255, 255, 255, particle_inferno);
}


void pLensFlareThink (cparticle_t *p, vec3_t org, vec3_t angle, float *alpha, float *size, int *image, float *time)
{
	angle[2] = anglemod(cl.refdef.viewangles[YAW]);	

	p->thinknext = true;
}

void CL_LensFlare(vec3_t pos, vec3_t color, float size, float time)
{
	setupParticle (
		0,	0,	0,
		pos[0], pos[1], pos[2],
		0, 0, 0,
		0, 0, 0,
		color[0], color[1], color[2],
		0,	0,	0,
		1,		- 2.0f/time,
		size,	0,			
		particle_lensflare,
		PART_LENSFLARE,
		pLensFlareThink, true);
}

/*
===============
CL_AddParticles
===============
*/

void CL_AddParticles (void)
{
	cparticle_t		*p, *next;
	float			alpha, size, light;
	float			time=0, time2;
	vec3_t			org, color, angle;
	int				i, image;
	cparticle_t		*active, *tail;

	active = NULL;
	tail = NULL;

	for (p=active_particles ; p ; p=next)
	{
		next = p->next;

		// PMM - added INSTANT_PARTICLE handling for heat beam
		if (p->alphavel != INSTANT_PARTICLE)
		{
			//this fixes jumpy particles
			if (cl.time>p->time)
				p->time = cl.time;

			time = (p->time - p->start)*0.001;
			alpha = p->alpha + time*p->alphavel;

			if (alpha <= 0)
			{	// faded out
				p->next = free_particles;
				free_particles = p;
				continue;
			}
		}
		else
		{
			alpha = p->alpha;
		}

		p->next = NULL;
		if (!tail)
			active = tail = p;
		else
		{
			tail->next = p;
			tail = p;
		}

		if (alpha > 1.0)
			alpha = 1;

		time2 = time*time;
		image = p->image;

		for (i=0;i<3;i++)
		{
			color[i] = p->color[i] + p->colorvel[i]*time;
			if (color[i]>255) color[i]=255;
			if (color[i]<0) color[i]=0;
			
			angle[i] = p->angle[i];
			org[i] = p->org[i] + p->vel[i]*time + p->accel[i]*time2;
		}

		if (p->flags&PART_GRAVITY)
			org[2]+=time2*-PARTICLE_GRAVITY;

		size = p->size + p->sizevel*time;

		for (i=0;i<P_LIGHTS_MAX;i++)
		{
			const cplight_t *plight = &p->lights[i];
			if (plight->isactive)
			{
				light = plight->light*alpha + plight->lightvel*time;
				V_AddLight (org, light, plight->lightcol[0], plight->lightcol[1], plight->lightcol[2]);
			}
		}

		if (p->thinknext && p->think)
		{
			p->thinknext=false;
			p->think(p, org, angle, &alpha, &size, &image, &time);
		}

		V_AddParticle (org, angle, color, alpha, size, image, p->flags);
		
		if (p->alphavel == INSTANT_PARTICLE)
		{
			p->alphavel = 0.0;
			p->alpha = 0.0;
		}
	}

	active_particles = active;
}


/*
==============
CL_EntityEvent

An entity has just been parsed that has an event value

the female events are there for backwards compatability
==============
*/
extern struct sfx_s	*cl_sfx_footsteps[4];

void CL_EntityEvent (entity_state_t *ent)
{
	switch (ent->event)
	{
	case EV_ITEM_RESPAWN:
		S_StartSound (NULL, ent->number, CHAN_WEAPON, S_RegisterSound("items/respawn1.wav"), 1, ATTN_IDLE, 0);
		CL_ItemRespawnParticles (ent->origin);
		break;
	case EV_PLAYER_TELEPORT:
		S_StartSound (NULL, ent->number, CHAN_WEAPON, S_RegisterSound("misc/tele1.wav"), 1, ATTN_IDLE, 0);
		CL_TeleportParticles (ent->origin);
		break;
	case EV_FOOTSTEP:
		if (cl_footsteps->value)
			S_StartSound (NULL, ent->number, CHAN_BODY, cl_sfx_footsteps[rand()&3], 1, ATTN_NORM, 0);
		break;
	case EV_FALLSHORT:
		S_StartSound (NULL, ent->number, CHAN_AUTO, S_RegisterSound ("player/land1.wav"), 1, ATTN_NORM, 0);
		break;
	case EV_FALL:
		S_StartSound (NULL, ent->number, CHAN_AUTO, S_RegisterSound ("*fall2.wav"), 1, ATTN_NORM, 0);
		break;
	case EV_FALLFAR:
		S_StartSound (NULL, ent->number, CHAN_AUTO, S_RegisterSound ("*fall1.wav"), 1, ATTN_NORM, 0);
		break;
	}
}


/*
==============
CL_ClearEffects

==============
*/
void CL_ClearEffects (void)
{
	CL_ClearParticles ();
	CL_ClearDlights ();
	CL_ClearLightStyles ();
}
