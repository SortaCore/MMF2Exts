#ifndef VOXEL_DATATYPES
#define VOXEL_DATATYPES


//#define MAX_LIMBS 32
//#define MAX_VOXEL_SIZE 128
#define M_RAD		57.2957795f	// matches value in gcc v2 math.h
//#define VPATCH_SIZE 114

typedef COLORREF pal_t[256];

typedef float vec_t;
typedef vec_t vec3_t[3];

//union funi {
//	char	c[4];
//	float	f;
//};


typedef struct matrix_t {
	float	val[3][4];
}matrix_t;

//typedef struct voxelColumn_t {
//	unsigned char	color[256];
//	char			normal[256];
//}voxelColumn_t;

typedef struct voxelDot_t {
	vec3_t			position;
	char			color;
	char			normal;
}voxelDot_t;

//typedef struct voxelPart_t {
//	voxelColumn_t	vox[MAX_VOXEL_SIZE][MAX_VOXEL_SIZE];
//	char			size[3];
//	float			scale[3];
//	char			name[16];
//	char			normalMode;
//	voxelDot_t		vDot[MAX_VOXEL_SIZE*MAX_VOXEL_SIZE];
//	int				vDotCount;
//	float			offset[3];
//	float			minBounds[3];
//	float			maxBounds[3];
//	matrix_t		animMatrix[128];
//	matrix_t		baseMatrix;
//}voxelPart_t;
//
//typedef struct voxel_t {
//	voxelPart_t	*part[MAX_LIMBS];
//	int			partCount;
//	pal_t		palette;
//	long		numFrames;
//	int			active;
//}voxel_t;

typedef struct vxl_runtime_part {
	char name[16]; /* ASCIIZ string - name of section */
	long n_dots;
	voxelDot_t * dots;
	matrix_t		animMatrix[128];

	char normals;
	float offset[3];
	float scale[3];
    float minbounds[3];
    float maxbounds[3];    /* Scaling vector for the image */

}vxl_runtime_part;

typedef struct vxl_header {
	char filetype[16];    /* ASCIIZ string - "Voxel Animation" */
	long unknown;         /* Always 1 - number of animation frames? */
	long n_limbs;         /* Number of limb headers/bodies/tailers */
	long n_limbs2;        /* Always the same as n_limbs */
	long bodysize;        /* Total size in bytes of all limb bodies */
	char unknown2;		  /* Always 0x1f10 - ID or end of header code? */
	char palette[256*3];  /* 256 colour palette for the voxel in RGB format */
}vxl_header;

typedef struct vxl_limb_header {
	char limb_name[16]; /* ASCIIZ string - name of section */
	long limb_number;   /* Limb number */
	long unknown;       /* Always 1 */
	long unknown2;      /* Always 0 */
}vxl_limb_header;

typedef struct vxl_limb_body{
	long * span_start; /* List of span start addresses or -1 */
    long * span_end;   /* List of span end addresses  or -1 */
}vxl_limb_body;

typedef struct vxl_limb_tailer {
    long  span_start_off;  /* Offset into body section to span start list */
    long  span_end_off;    /* Offset into body section to span end list */
    long  span_data_off;   /* Offset into body section to span data */
	float det;
	float transform[3][4];
    float minbounds[3];
    float maxbounds[3];    /* Scaling vector for the image */
    char  xsize;           /* Width of the voxel limb */
    char  ysize;           /* Breadth of the voxel limb */
    char  zsize;           /* Height of the voxel limb */
    char  unknown;         /* Always 2 - unknown */ /* NORMALS MODE for model*/
}vxl_limb_tailer;

#endif