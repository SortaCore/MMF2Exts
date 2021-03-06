
#ifndef VOXEL_H
#define VOXEL_H

#include "DataTypes.h"
#include "Pallette.h"
#include "Normals.h"
#include <cmath>





void GenerateMatrix( vec3_t angle, matrix_t *matrix, LPRDATA rdPtr ){
	rdPtr->sinus[0] = sin(angle[0] / M_RAD);
	rdPtr->sinus[1] = sin(angle[1] / M_RAD);
	rdPtr->sinus[2] = sin(angle[2] / M_RAD);
	rdPtr->cosine[0] = cos(angle[0] / M_RAD);
	rdPtr->cosine[1] = cos(angle[1] / M_RAD);
	rdPtr->cosine[2] = cos(angle[2] / M_RAD);

	matrix->val[0][0] = -rdPtr->cosine[0]*rdPtr->cosine[2];
	matrix->val[0][1] = -rdPtr->cosine[0]*rdPtr->sinus[2];
	matrix->val[0][2] = rdPtr->sinus[0];

	matrix->val[1][0] = -rdPtr->sinus[1]*rdPtr->sinus[0]*rdPtr->cosine[2]+rdPtr->cosine[1]*rdPtr->sinus[2];
	matrix->val[1][1] = -rdPtr->sinus[1]*rdPtr->sinus[0]*rdPtr->sinus[2]-rdPtr->cosine[1]*rdPtr->cosine[2];
	matrix->val[1][2] = -rdPtr->cosine[0]*rdPtr->sinus[1];

	matrix->val[2][0] = -rdPtr->cosine[1]*rdPtr->sinus[0]*rdPtr->cosine[2]-rdPtr->sinus[1]*rdPtr->sinus[2];
	matrix->val[2][1] = -rdPtr->cosine[1]*rdPtr->sinus[0]*rdPtr->sinus[2]+rdPtr->sinus[1]*rdPtr->cosine[2];
	matrix->val[2][2] = -rdPtr->cosine[0]*rdPtr->cosine[1];

	//FILE* debug = fopen("c:\\debug-matrix.txt", "w");

	//fprintf( debug, "[%.3f][%.3f][%.3f]\n[%.3f][%.3f][%.3f]\n[%.3f][%.3f][%.3f]\n", matrix->val[0][0], matrix->val[0][1], matrix->val[0][2], matrix->val[1][0], matrix->val[1][1], matrix->val[1][2], matrix->val[2][0], matrix->val[2][1], matrix->val[2][2] );
	//fclose(debug);
}

void MatrixMultiply(matrix_t *matrix1, matrix_t *matrix2, matrix_t *matrixOut) {
	matrixOut->val[0][0] = matrix1->val[0][0] * matrix2->val[0][0] + matrix1->val[0][1] * matrix2->val[1][0] +
				matrix1->val[0][2] * matrix2->val[2][0];
	matrixOut->val[0][1] = matrix1->val[0][0] * matrix2->val[0][1] + matrix1->val[0][1] * matrix2->val[1][1] +
				matrix1->val[0][2] * matrix2->val[2][1];
	matrixOut->val[0][2] = matrix1->val[0][0] * matrix2->val[0][2] + matrix1->val[0][1] * matrix2->val[1][2] +
				matrix1->val[0][2] * matrix2->val[2][2];
	matrixOut->val[1][0] = matrix1->val[1][0] * matrix2->val[0][0] + matrix1->val[1][1] * matrix2->val[1][0] +
				matrix1->val[1][2] * matrix2->val[2][0];
	matrixOut->val[1][1] = matrix1->val[1][0] * matrix2->val[0][1] + matrix1->val[1][1] * matrix2->val[1][1] +
				matrix1->val[1][2] * matrix2->val[2][1];
	matrixOut->val[1][2] = matrix1->val[1][0] * matrix2->val[0][2] + matrix1->val[1][1] * matrix2->val[1][2] +
				matrix1->val[1][2] * matrix2->val[2][2];
	matrixOut->val[2][0] = matrix1->val[2][0] * matrix2->val[0][0] + matrix1->val[2][1] * matrix2->val[1][0] +
				matrix1->val[2][2] * matrix2->val[2][0];
	matrixOut->val[2][1] = matrix1->val[2][0] * matrix2->val[0][1] + matrix1->val[2][1] * matrix2->val[1][1] +
				matrix1->val[2][2] * matrix2->val[2][1];
	matrixOut->val[2][2] = matrix1->val[2][0] * matrix2->val[0][2] + matrix1->val[2][1] * matrix2->val[1][2] +
				matrix1->val[2][2] * matrix2->val[2][2];
}

void MatrixRotate( vec3_t output, matrix_t *matrix, vec3_t input ){
	/*FILE * debug = fopen("c:\\matrix-rotate-vc.txt", "w");
	fprintf(debug, "input : %8.2f%8.2f%8.2f\n", input[0], input[1], input[2]);fflush(debug);
	fprintf(debug, "matrix: %8.2f%8.2f%8.2f\n", matrix->val[0][0], matrix->val[1][0], matrix->val[2][0]);
	fprintf(debug, "matrix: %8.2f%8.2f%8.2f\n", matrix->val[0][1], matrix->val[1][1], matrix->val[2][1]);
	fprintf(debug, "matrix: %8.2f%8.2f%8.2f\n", matrix->val[0][2], matrix->val[1][2], matrix->val[2][2]);
	fprintf(debug, "matrix: %8.2f%8.2f%8.2f\n", matrix->val[0][3], matrix->val[1][3], matrix->val[2][3]);fflush(debug);*/
	output[0] = input[0] * matrix->val[0][0] + input[1] * matrix->val[0][1] + input[2] * matrix->val[0][2] + matrix->val[0][3];
	output[1] = input[0] * matrix->val[1][0] + input[1] * matrix->val[1][1] + input[2] * matrix->val[1][2] + matrix->val[1][3];
	output[2] = input[0] * matrix->val[2][0] + input[1] * matrix->val[2][1] + input[2] * matrix->val[2][2] + matrix->val[2][3];
	/*fprintf(debug, "output: %8.2f%8.2f%8.2f\n\n\n\n\n\n", output[0], output[1], output[2]);fflush(debug);
	fclose(debug);*/
}

void VectorCopy( const vec3_t in, vec3_t out ) {
	out[0] = in[0];
	out[1] = in[1];
	out[2] = in[2];
}

void voxel_Set_part(int vpart, int frame, LPRDATA rdPtr){
	vec3_t		point,destpoint;
	int			x,y;
	// char			colorindex;
	voxelDot_t *	Dot;
	matrix_t		out, *in, *positioningMatrix;

	in = &rdPtr->voxelParts[vpart]->animMatrix[frame];

	#ifdef VOXEL_DEBUG
	FILE* debug;
	debug = fopen("c:\\debug-set-part.txt", "w");
	fprintf(debug, "#########surfaceDepthGrid##########\n\n");
	fflush(debug);
	#endif

	if(0){
		memset( &out, 0, sizeof( matrix_t ) );

		point[0] = in->val[0][3];
		point[1] = in->val[1][3];
		point[2] = in->val[2][3];

		MatrixMultiply(&rdPtr->lMatrix, in, &out);

		#ifdef VOXEL_DEBUG
		fprintf(debug, "#########matrix multiply: ok##########\n\n");
		fflush(debug);
		#endif

		MatrixRotate( destpoint, &rdPtr->lMatrix, point );

		#ifdef VOXEL_DEBUG
		fprintf(debug, "#########matrix rotate: ok##########\n\n");
		fflush(debug);
		#endif

		out.val[0][3] = destpoint[0];
		out.val[1][3] = destpoint[1];
		out.val[2][3] = destpoint[2];

		#ifdef VOXEL_DEBUG
		fprintf(debug, "#########assigning of points: ok##########\n\n");
		fflush(debug);

		fprintf(debug, "%5i%5i\n", vpart, frame);
		fprintf(debug, "point: %8.2f%8.2f%8.2f	surfaceDepthGrid: %i\n", point[0], point[1], point[2], -1);// rdPtr->surfaceDepthGrid[x * rdPtr->rHo.hoImgHeight + y]);
		fflush(debug);
		fprintf(debug, "DESTpoint: %8.2f%8.2f%8.2f	surfaceDepthGrid: %i\n", destpoint[0], destpoint[1], destpoint[2], -1);// rdPtr->surfaceDepthGrid[x * rdPtr->rHo.hoImgHeight + y]);
		fflush(debug);

		fprintf(debug, "pass(dotcount: %i):\n\n", rdPtr->voxelParts[vpart]->n_dots);
		fflush(debug);
		#endif

		positioningMatrix = &out;
	} else {
		positioningMatrix = &rdPtr->lMatrix;
	}

	for(int i = 0; i < rdPtr->voxelParts[vpart]->n_dots; i++){
		Dot = &rdPtr->voxelParts[vpart]->dots[i];

		#ifdef VOXEL_DEBUG
		fprintf(debug, "scale: %8.2f %8.2f %8.2f\n", rdPtr->voxelParts[vpart]->scale[0], rdPtr->voxelParts[vpart]->scale[1], rdPtr->voxelParts[vpart]->scale[2]);
		fprintf(debug, "maxbounds: %8.2f %8.2f %8.2f\n", rdPtr->voxelParts[vpart]->maxbounds[0], rdPtr->voxelParts[vpart]->maxbounds[1], rdPtr->voxelParts[vpart]->maxbounds[2]);
		fflush(debug);
		#endif

		destpoint[0] = Dot->position[0] /** rdPtr->voxelParts[vpart]->scale[0]*/ - rdPtr->voxelParts[vpart]->maxbounds[0];
		destpoint[1] = Dot->position[1] /** rdPtr->voxelParts[vpart]->scale[1]*/ - rdPtr->voxelParts[vpart]->maxbounds[1];
		destpoint[2] = Dot->position[2] /** rdPtr->voxelParts[vpart]->scale[2]*/ - rdPtr->voxelParts[vpart]->maxbounds[2];

		#ifdef VOXEL_DEBUG
		fprintf(debug, "destpoint: %8.2f %8.2f %8.2f\tDot->position: %8.2f %8.2f %8.2f\n", destpoint[0], destpoint[1], destpoint[2], Dot->position[0], Dot->position[1], Dot->position[2]);
		fflush(debug);
		#endif

		MatrixRotate(point, positioningMatrix, destpoint);

		#ifdef VOXEL_DEBUG
		fprintf(debug, "resulting point: %8.2f %8.2f %8.2f\n", point[0], point[1], point[2]);
		fflush(debug);
		#endif

		x = (int)(rdPtr->rHo.hoImgWidth / 2.0f + point[0]);
		y = (int)(rdPtr->rHo.hoImgHeight / 2.0f + point[1]);

		#ifdef VOXEL_DEBUG
		fprintf(debug, "resulting x/y: %d, %d\n", x, y);
		fflush(debug);
		#endif

		rdPtr->screenRender[x * rdPtr->rHo.hoImgHeight + y] = x==max(0,min(rdPtr->rHo.hoImgWidth-1,x)) && y==max(0,min(rdPtr->rHo.hoImgHeight-1,y));

		if(-point[2] <= rdPtr->surfaceDepthGrid[x * rdPtr->rHo.hoImgHeight + y]){
			rdPtr->surfaceDepthGrid[x * rdPtr->rHo.hoImgHeight + y] = (short)-point[2];
			rdPtr->screenDot[x * 3 * rdPtr->rHo.hoImgHeight + y * 3 + 0] = Dot->color;
			rdPtr->screenDot[x * 3 * rdPtr->rHo.hoImgHeight + y * 3 + 1] = Dot->normal;
		}

		//if(x!=max(0,min(rdPtr->rHo.hoImgWidth-1,x)) || y!=max(0,min(rdPtr->rHo.hoImgHeight-1,y)))
	}

	#ifdef VOXEL_DEBUG
	fclose(debug);
	#endif
}

vec_t VectorNormalize( vec3_t v ) {
	float	length, ilength;

	length = v[0]*v[0] + v[1]*v[1] + v[2]*v[2];
	length = sqrt (length);

	if ( length ) {
		ilength = 1/length;
		v[0] *= ilength;
		v[1] *= ilength;
		v[2] *= ilength;
	}

	return length;
}

void VectorScale( const vec3_t in, vec_t scale, vec3_t out ) {
	out[0] = in[0]*scale;
	out[1] = in[1]*scale;
	out[2] = in[2]*scale;
}

void RotateZ( vec3_t output, vec3_t input, LPRDATA rdPtr ){
	output[0] = (rdPtr->cosine[2] * input[0]) - (rdPtr->sinus[2] * input[1]);
	output[1] = (rdPtr->sinus[2] * input[0]) + (rdPtr->cosine[2] * input[1]);
	output[2] = input[2];
}

void RotateY( vec3_t output, vec3_t input, LPRDATA rdPtr ){
	output[0] = (rdPtr->cosine[1] * input[0]) + (rdPtr->sinus[1] * input[2]);
	output[1] = input[1];
	output[2] = -(rdPtr->sinus[1] * input[0]) + (rdPtr->cosine[1] * input[2]);
}

void RotateX( vec3_t output, vec3_t input, LPRDATA rdPtr ){
	output[0] = input[0];
	output[1] = (rdPtr->cosine[0] * input[1]) - (rdPtr->sinus[0] * input[2]);
	output[2] = (rdPtr->sinus[0] * input[1]) + (rdPtr->cosine[0] * input[2]);
}

vec_t DotProduct( const vec3_t v1, const vec3_t v2 ) {
	return v1[0]*v2[0] + v1[1]*v2[1] + v1[2]*v2[2];
}

void voxel_Render_unit(LPRDATA rdPtr){
	cSurface * targetSurface = rdPtr->pSf;

	int		i,j;
	float	lightmod = 1.0F;
	vec3_t	lightvec,halfvec,specvec;
	vec3_t	normal;
	float	specular,ambient;
	float	r,g,b;
	unsigned char	colorindex;
	char	normalindex;
	vec3_t	destpoint;

	#ifdef VOXEL_DEBUG
	FILE* debug = fopen("debug-render-unit.txt", "w");
	#endif

	if(rdPtr->enableShading){
		specular = rdPtr->specular;
		ambient = rdPtr->ambient;

		//lightvec[0] = rdPtr->xLight;
		//lightvec[1] = rdPtr->yLight;
		//lightvec[2] = rdPtr->zLight;

		lightvec[0] = -1.0f;
		lightvec[1] = 1.0f;
		lightvec[2] = -1.0f;

		specvec[0] = -0.4f;
		specvec[1] = 1.0f;
		specvec[2] = -0.2f;

		//halfvec[0] = -68f;// rdPtr->xLight;
		//halfvec[1] = 35f;// rdPtr->yLight;
		//halfvec[2] = -300f;// rdPtr->zLight;

		halfvec[0] = 33.33f;
		halfvec[1] = 0.0f;
		halfvec[2] = 66.66f;

		VectorNormalize(lightvec);
		VectorNormalize(halfvec);
		VectorNormalize(specvec);
	}

	for( i = 0; i < rdPtr->rHo.hoImgWidth; i++ ){
		for( j = 0; j < rdPtr->rHo.hoImgHeight; j++ ){
			#ifdef VOXEL_DEBUG
			fprintf(debug, "surfaceDepthGrid[%d][%d] = %d\n", i, j, rdPtr->surfaceDepthGrid[i * rdPtr->rHo.hoImgHeight + j]);
			#endif

			int iHeight = i * rdPtr->rHo.hoImgHeight;

			if(rdPtr->surfaceDepthGrid[iHeight + j] < 1000 ){
				int iHeight3 = iHeight * 3;
				colorindex = rdPtr->screenDot[iHeight3 + j * 3 + 0];
				normalindex= rdPtr->screenDot[iHeight3 + j * 3 + 1];
				if(rdPtr->screenRender[iHeight + j]) {
					if(rdPtr->enableShading){
						if( rdPtr->voxelParts[0]->normals == 2 ){
							VectorCopy( TSnormals[normalindex], destpoint );
						} else {
							VectorCopy( RA2normals[normalindex], destpoint );
						}

						VectorScale( destpoint, 10, destpoint );

						RotateZ( normal, destpoint, rdPtr );
						destpoint[0] = normal[0];
						destpoint[1] = normal[1];
						destpoint[2] = normal[2];
						RotateY( normal, destpoint, rdPtr );
						destpoint[0] = normal[0];
						destpoint[1] = normal[1];
						destpoint[2] = normal[2];
						RotateX( normal, destpoint, rdPtr );

						VectorNormalize(normal);

						lightmod = DotProduct(lightvec, normal);

						#ifdef VOXEL_DEBUG
						fprintf(debug, "index: %d\tnormal: %8.2f %8.2f %8.2f lightmod: %8.2f\n", normalindex, normal[0], normal[1], normal[2], lightmod);
						#endif

						lightmod /= 1 / ambient;
						lightmod += ambient;

						if(lightmod < ambient) lightmod = ambient;
						if(lightmod > 1.0) lightmod = 1.0;
					}

					b = ((ts_palette[colorindex] & 0xFF0000) >> 16) * lightmod;
					g = ((ts_palette[colorindex] & 0xFF00) >> 8) * lightmod;
					r = (ts_palette[colorindex] & 0xFF) * lightmod;

					if(rdPtr->enableShading){
						if(1){
							halfvec[0] = specvec[0] / 2 + halfvec[0] / 2;
							halfvec[1] = specvec[1] / 2 + halfvec[1] / 2;
							halfvec[2] = specvec[2] / 2 + halfvec[2] / 2;

							lightmod = DotProduct( halfvec, normal );
							lightmod *= lightmod * lightmod * 255 * specular;

							if( lightmod < 0.0 )lightmod = 0.0;

							r += lightmod;
							g += lightmod;
							b += lightmod;
						}
					}

					r=max(1,min(250,r));
					g=max(1,min(250,g));
					b=max(1,min(250,b));

					targetSurface->SetPixel(j, i, RGB(r, g, b));
				}
			}
		}
	}
	#ifdef VOXEL_DEBUG
	fclose(debug);
	#endif
}

void renderVoxel(LPRDATA rdPtr){
	do{
		vec3_t	cam;

		cam[0] = (vec_t)rdPtr->xAngle;
		cam[1] = (vec_t)rdPtr->yAngle;
		cam[2] = (vec_t)rdPtr->zAngle;

		GenerateMatrix(cam, &rdPtr->lMatrix, rdPtr );

		if(1){// rdPtr->voxel.active){
			memset(rdPtr->surfaceDepthGrid, 100, rdPtr->rHo.hoImgWidth * rdPtr->rHo.hoImgHeight * 2);

			for(int i = 0 ; i < rdPtr->n_parts; i ++ ){
				voxel_Set_part(i,1/*rdPtr->frame % rdPtr->voxel.numFrames*/, rdPtr);
			}

			rdPtr->pSf->Fill(RGB(0,0,0));

			voxel_Render_unit(rdPtr);
		}
	}while(FALSE);
}








vxl_header * loadVoxelHeader(char * fileContents){
	vxl_header * voxelHeader = new vxl_header;

	memcpy(voxelHeader, fileContents, sizeof(vxl_header));

	#ifdef VOXEL_DEBUG
	FILE * debug = fopen("load-voxel-header.debug.log", "w");
	fprintf(debug, "size of header: %d\n\n", sizeof(vxl_header));
	fprintf(debug, "filetype:\t%s\nunknown:\t%d\nn_limbs:\t%d\nn_limbs2:\t%d\nbodysize:\t%d\nunknown2:\t0x%02X\npalette:\tnot important...?!...!!\n\n", voxelHeader->filetype, voxelHeader->unknown, voxelHeader->n_limbs, voxelHeader->n_limbs2, voxelHeader->bodysize, voxelHeader->unknown2);
	fflush(debug);
	fclose(debug);
	#endif

	return voxelHeader;
}

char * getFileContents(char * filePath){
	FILE * source = fopen(filePath, "rb");

	if(!source){
		return 0;
	}

	fseek(source, 0, SEEK_END);
	long fileSize = ftell(source);

	char * fileContents = (char *)malloc(fileSize+1);

	fseek(source, 0, SEEK_SET);
	fread(fileContents, 1, fileSize, source);
	fclose(source);

	return fileContents;
}

vxl_limb_header * loadVoxelLimbHeaders(char * fileContents, vxl_header * voxelHeader){
	vxl_limb_header * voxelLimbHeaders = new vxl_limb_header[voxelHeader->n_limbs];

	memcpy(voxelLimbHeaders, fileContents + 802, sizeof(vxl_limb_header) * voxelHeader->n_limbs);

	#ifdef VOXEL_DEBUG
	FILE * debug = fopen("load-voxel-limb-headers.debug.log", "w");
	fprintf(debug, "size of each record: %d\nnumber of limbs: %i\n\n", sizeof(vxl_limb_header), voxelHeader->n_limbs);

	for(int i = 0; i < voxelHeader->n_limbs; i++){
		fprintf(debug, "limbname:\t%s\nlimb_number:\t%ld\t%02X\nunknown:\t%ld\nunknown2:\t%ld\n\n", voxelLimbHeaders[i].limb_name, voxelLimbHeaders[i].limb_number, voxelLimbHeaders[i].limb_number, voxelLimbHeaders[i].unknown, voxelLimbHeaders[i].unknown2);
		fflush(debug);
	}

	fclose(debug);
	#endif

	return voxelLimbHeaders;
}

vxl_limb_tailer * loadVoxelLimbTailers(char * fileContents, vxl_header * voxelHeader){
	vxl_limb_tailer * voxelLimbTailers = new vxl_limb_tailer[voxelHeader->n_limbs];

	memcpy(voxelLimbTailers, fileContents + 802 + sizeof(vxl_limb_header) * voxelHeader->n_limbs + voxelHeader->bodysize, sizeof(vxl_limb_tailer) * voxelHeader->n_limbs);

	//#ifdef VOXEL_DEBUG
	//FILE * debug = fopen("load-voxel-limb-tailers.debug.log", "w");
	//fprintf(debug, "number of limbs: %i\n\n", voxelHeader->n_limbs);

	//for(int i = 0; i < voxelHeader->n_limbs; i++){
	//	fprintf(debug, "span_start_off:\t%d\nspan_end_off:\t%d\nspan_data_off:\t%d\nscale:\t{%8.2f, %8.2f, %8.2f}\nxSize:\t%d\nySize:\t%d\nzSize:\t%d\nunknown:\t%d\n\n", voxelLimbTailers[i].span_start_off,  voxelLimbTailers[i].span_end_off,  voxelLimbTailers[i].span_data_off,  voxelLimbTailers[i].scale[0],  voxelLimbTailers[i].scale[1],  voxelLimbTailers[i].scale[2], voxelLimbTailers[i].xsize,  voxelLimbTailers[i].ysize,  voxelLimbTailers[i].zsize,  voxelLimbTailers[i].unknown);
	//	fflush(debug);
	//}

	//fclose(debug);
	//#endif

	return voxelLimbTailers;
}

long * loadLimbBodySpanStartOffsets(char * fileContents, vxl_header * voxelHeader, vxl_limb_tailer * voxelLimbTailer){
	#ifdef VOXEL_DEBUG
	FILE * debug = fopen("load-voxel-limb-body-span-start-offset.debug.log", "w");
	#endif

	int xyGridCount = voxelLimbTailer->xsize * voxelLimbTailer->ysize;

	int contentOffset = 802 + sizeof(vxl_limb_header) * voxelHeader->n_limbs + voxelLimbTailer->span_start_off;
	int length = sizeof(vxl_limb_body) * xyGridCount;

	long * limbBody = new long[length];

	#ifdef VOXEL_DEBUG
	fprintf(debug, "Offset for VXL: %ld, bytesToRead: %d, size of limbBody: %d\n\n", contentOffset, length, sizeof(long) * xyGridCount);
	fflush(debug);
	#endif

	memcpy(limbBody, fileContents + contentOffset, length);

	#ifdef VOXEL_DEBUG
	for(int i = 0; i < xyGridCount; i++){
		if(i == 0){
			fprintf(debug, "span_start: %ld", limbBody[i]);
		} else {
			fprintf(debug, ", %ld", limbBody[i]);
		}
		fflush(debug);
	}

	fprintf(debug, "\n\n");
	fclose(debug);
	#endif

	return limbBody;
}

long * loadLimbBodySpanEndOffsets(char * fileContents, vxl_header * voxelHeader, vxl_limb_tailer * voxelLimbTailer){
	#ifdef VOXEL_DEBUG
	FILE * debug = fopen("load-voxel-limb-body-span-end-offset.debug.log", "w");
	#endif

	int xyGridCount = voxelLimbTailer->xsize * voxelLimbTailer->ysize;

	int contentOffset = 802 + sizeof(vxl_limb_header) * voxelHeader->n_limbs + voxelLimbTailer->span_end_off;
	int length = sizeof(long) * xyGridCount;

	long * limbBody = new long[length];

	#ifdef VOXEL_DEBUG
	fprintf(debug, "Offset for VXL: %ld, bytesToRead: %d, size of limbBody: %d\n\n", contentOffset, length, sizeof(*limbBody));
	fflush(debug);
	#endif

	memcpy(limbBody, fileContents + contentOffset, length);

	#ifdef VOXEL_DEBUG
	for(int i = 0; i < xyGridCount; i++){
		if(i == 0){
			fprintf(debug, "span_start: %ld", limbBody[i]);
		} else {
			fprintf(debug, ", %ld", limbBody[i]);
		}
		fflush(debug);
	}

	fprintf(debug, "\n\n");
	fclose(debug);
	#endif

	return limbBody;
}

char * loadVoxelPart(int partNumber, char * fileContents, vxl_header * voxelHeader, vxl_limb_header * voxelLimbHeader, vxl_limb_tailer * voxelLimbTailer){
	int xyElementCount = voxelLimbTailer->xsize * voxelLimbTailer->ysize;

	#ifdef VOXEL_DEBUG
	FILE * debug = fopen("load-voxel-part.debug.log", "w");
	fprintf(debug, "Loading limb: %d\n\n", partNumber);
	fprintf(debug, "Dimensions: %d, %d, %d\n", voxelLimbTailer->xsize, voxelLimbTailer->ysize, voxelLimbTailer->zsize);
	fflush(debug);
	#endif

	#ifdef VOXEL_DEBUG
	fprintf(debug, "Loading span_start_offsets\n");
	fflush(debug);
	#endif

	long * limbBodySpanStartOffsets = loadLimbBodySpanStartOffsets(fileContents, voxelHeader, voxelLimbTailer);

	#ifdef VOXEL_DEBUG
	fprintf(debug, "Loading span_end_offsets\n");
	fflush(debug);
	#endif

	long * limbBodySpanEndOffsets = loadLimbBodySpanEndOffsets(fileContents, voxelHeader, voxelLimbTailer);

	char * part = new char[2 * xyElementCount * voxelLimbTailer->zsize];
	memset (part, 255, 2 * xyElementCount * voxelLimbTailer->zsize);

	#ifdef VOXEL_DEBUG
	fprintf(debug, "Allocated memory for part (middlelayer): %d bytes (pointing to %p)\n", 2 * xyElementCount * voxelLimbTailer->zsize, part);
	fflush(debug);
	#endif



	for(int i = 0 ; i < xyElementCount; i++ ){
		//int x = i % voxelLimbTailer->xsize;
		//int y = i / voxelLimbTailer->xsize;

		int body_position = 802 + sizeof(vxl_limb_header) * voxelHeader->n_limbs;
		int span_data_offset = voxelLimbTailer->span_data_off;
		int span_start_offset = limbBodySpanStartOffsets[i];
		int span_end_offset = limbBodySpanEndOffsets[i];
		int span_length = span_end_offset - span_start_offset;

		#ifdef VOXEL_DEBUG
			fprintf(debug, "Position\n");
			fflush(debug);
		#endif

		if( span_start_offset != -1 && span_end_offset != -1 ){
			long span_start = body_position + span_data_offset + span_start_offset;
			long span_end = body_position + span_data_offset + span_end_offset;

			int readingPosition = span_start;

			int z_position = 0;

			while(readingPosition < span_end){
				char skipVoxels = fileContents[readingPosition++];
				char voxelCount = fileContents[readingPosition++];

				z_position += skipVoxels;

				#ifdef VOXEL_DEBUG
				fprintf(debug, /*"spanStart: %d + %d, "*/ "skipVoxels: %d, voxelCount: %d, voxels left in span: %d\n", /*span_data_offset, span_start_offset, */skipVoxels, voxelCount, voxelLimbTailer->zsize - voxelCount);
				fflush(debug);
				#endif

				for(int voxelNumber = skipVoxels; voxelNumber < skipVoxels + voxelCount; voxelNumber++){
					char colorIndex = fileContents[readingPosition++];
					char normal = fileContents[readingPosition++];

					COLORREF color = ts_palette[colorIndex];

					#ifdef VOXEL_DEBUG
					fprintf(debug, "v%d: color: %5i %02X, resulting color: %08X, storing address: %d\n", voxelNumber + skipVoxels, colorIndex, colorIndex, color, 2 * (i * voxelLimbTailer->zsize + voxelNumber + skipVoxels));
					fflush(debug);
					#endif

					part[2 * (i * voxelLimbTailer->zsize + z_position)] = colorIndex;
					part[2 * (i * voxelLimbTailer->zsize + z_position) + 1] = normal;

					z_position++;
				}

				char voxelCount2 = fileContents[readingPosition++];

				if(voxelCount != voxelCount2){
					#ifdef VOXEL_DEBUG
					fprintf(debug, "Voxel count mismatch! %d != %d\n", voxelCount, voxelCount2);
					fflush(debug);
					#endif
				}

				#ifdef VOXEL_DEBUG
				fprintf(debug, "Voxel count: %d.\n", voxelCount);
				fflush(debug);
				#endif
			}
		}
	}

	delete [] limbBodySpanStartOffsets;
	delete [] limbBodySpanEndOffsets;

	#ifdef VOXEL_DEBUG
	fclose(debug);
	#endif

	return part;
}

vxl_runtime_part ** loadVoxelParts(char * fileContents, vxl_header * voxelHeader, vxl_limb_header * voxelLimbHeaders, vxl_limb_tailer * voxelLimbTailers){
	#ifdef VOXEL_DEBUG
	FILE * debug = fopen("load-voxel-parts.debug.log", "w");
	fprintf(debug, "Number of limbs: %d\n\n", voxelHeader->n_limbs);
	fflush(debug);
	#endif

	char ** loadedVoxelParts = new char*[voxelHeader->n_limbs];

	for(int i = 0; i < voxelHeader->n_limbs; i++){
		loadedVoxelParts[i] = NULL;
	}

	for(int i = 0; i < voxelHeader->n_limbs; i++){
		#ifdef VOXEL_DEBUG
		fprintf(debug, "Reading limb %d\n", i);
		fflush(debug);
		#endif

		loadedVoxelParts[i] = loadVoxelPart(i, fileContents, voxelHeader, &voxelLimbHeaders[i], &voxelLimbTailers[i]);
	}

	vxl_runtime_part ** resultingParts = new vxl_runtime_part*[voxelHeader->n_limbs];

	for(int i = 0; i < voxelHeader->n_limbs; i++){
		resultingParts[i] = new vxl_runtime_part;

		//resultingParts[i]->scale = voxelLimbTailers[i].scale;
		//resultingParts[i]->scale = voxelLimbTailers[i].;

		resultingParts[i]->normals = voxelLimbTailers[i].unknown;

		resultingParts[i]->scale[0] = 1;// voxelLimbTailers[i].xsize / ( resultingParts[i]->maxbounds[2] - resultingParts[i]->minbounds[2] );
		resultingParts[i]->scale[1] = 1;// voxelLimbTailers[i].ysize / ( resultingParts[i]->maxbounds[0] - resultingParts[i]->minbounds[0] );
		resultingParts[i]->scale[2] = 1;// voxelLimbTailers[i].zsize / ( resultingParts[i]->maxbounds[1] - resultingParts[i]->minbounds[1] );

		#ifdef VOXEL_DEBUG
			fprintf(debug, "scale[%d] (%fd) = %d / (%fd - %fd)\n\n", 0, resultingParts[i]->scale[0], voxelLimbTailers[i].xsize, resultingParts[i]->maxbounds[2], resultingParts[i]->minbounds[2]);
			fflush(debug);
			fprintf(debug, "scale[%d] (%fd) = %d / (%fd - %fd)\n\n", 1, resultingParts[i]->scale[1], voxelLimbTailers[i].ysize, resultingParts[i]->maxbounds[0], resultingParts[i]->minbounds[0]);
			fflush(debug);
			fprintf(debug, "scale[%d] (%fd) = %d / (%fd - %fd)\n\n", 2, resultingParts[i]->scale[2], voxelLimbTailers[i].zsize, resultingParts[i]->maxbounds[1], resultingParts[i]->minbounds[1]);
			fflush(debug);
		#endif

		resultingParts[i]->offset[0] = resultingParts[i]->maxbounds[0] + resultingParts[i]->minbounds[0];
		resultingParts[i]->offset[1] = resultingParts[i]->maxbounds[1] + resultingParts[i]->minbounds[1];
		resultingParts[i]->offset[2] = resultingParts[i]->maxbounds[2] + resultingParts[i]->minbounds[2];

		char * source = loadedVoxelParts[i];

		#ifdef VOXEL_DEBUG
		fprintf(debug, "Source: %p\n", source);
		fflush(debug);
		#endif

		int ySize = voxelLimbTailers[i].ysize;
		int xSize = voxelLimbTailers[i].xsize;
		int zSize = voxelLimbTailers[i].zsize;

		int optimizedSize = 0;

		for(int y = 0; y < ySize; y++){
			for(int x = 0; x < xSize; x++){
				for(int z = 0; z < zSize; z++){
					int xyz = y * xSize * zSize + x * zSize + z;
					int sourcePosition = 2 * xyz;

					char colorIndex = source[sourcePosition];

					if(colorIndex != 0xFF){
						optimizedSize++;
					}
				}
			}
		}

		resultingParts[i]->n_dots = optimizedSize;
		resultingParts[i]->dots = new voxelDot_t[optimizedSize];

		#ifdef VOXEL_DEBUG
		fprintf(debug, "Size of source (before optimizing): %d, size of source (after optimizing): %d\n\n", ySize * xSize * zSize, optimizedSize);
		fflush(debug);
		#endif

		int arrayIndex = 0;

		for(int y = 0; y < ySize; y++){
			for(int x = 0; x < xSize; x++){
				for(int z = 0; z < zSize; z++){
					int xyz = y * xSize * zSize + x * zSize + z;
					int sourcePosition = 2 * xyz;

					char colorIndex = source[sourcePosition];
					char normalIndex = source[sourcePosition + 1];

					long resultingColor = ts_palette[colorIndex];

					//int left;
					//int right;
					//int top;
					//int bottom;

					if(colorIndex == 0xFF){
						continue;
					}

					#ifdef VOXEL_DEBUG
					fprintf(debug, "source[%04d] (color: %02X / %08X, x: %03d, y: %03d, z: %03d) = resultingDots[%d]", sourcePosition, colorIndex, resultingColor, x, y, z, arrayIndex);
					fflush(debug);
					#endif

					resultingParts[i]->dots[arrayIndex].position[0] = (float)x;
					resultingParts[i]->dots[arrayIndex].position[1] = (float)y;
					resultingParts[i]->dots[arrayIndex].position[2] = (float)z;
					resultingParts[i]->dots[arrayIndex].color = colorIndex;
					resultingParts[i]->dots[arrayIndex].normal = normalIndex;

					arrayIndex++;

					#ifdef VOXEL_DEBUG
					fprintf(debug, "(OK)\n");
					fflush(debug);
					#endif
				}
			}
		}
	}

	#ifdef VOXEL_DEBUG
	fclose(debug);
	#endif

	delete [] loadedVoxelParts;

	return resultingParts;
}

void loadVoxel(LPRDATA rdPtr, char * filepath){
	char * fileContents = getFileContents(filepath);

	#ifdef VOXEL_DEBUG
	FILE * debug = fopen("load-voxel.debug.log", "w");
	#endif

	if(fileContents == 0){
		#ifdef VOXEL_DEBUG
		fprintf(debug, "The file could not be found: %s", filepath); fflush(debug);
		#endif
		return;
	}

	vxl_header * voxelHeader = loadVoxelHeader(fileContents);

	rdPtr->n_parts = (char)voxelHeader->n_limbs;

	vxl_limb_header * voxelLimbHeaders = loadVoxelLimbHeaders(fileContents, voxelHeader);

	vxl_limb_tailer * voxelLimbTailers = loadVoxelLimbTailers(fileContents, voxelHeader);

	#ifdef VOXEL_DEBUG
	fprintf(debug, "Load parts...\n");
	fflush(debug);
	#endif

	vxl_runtime_part ** voxelParts = loadVoxelParts(fileContents, voxelHeader, voxelLimbHeaders, voxelLimbTailers);

	delete voxelHeader;
	delete [] voxelLimbHeaders;
	delete [] voxelLimbTailers;

	rdPtr->voxelParts = voxelParts;

	//#ifdef VOXEL_DEBUG
	//fprintf(debug, "Start iterating through parts...\n");
	//fflush(debug);
	//#endif

	//for(int i = 0; i < voxelLimbHeaders[0].unknown2; i++){
	//	voxelDot_t dot = voxelParts[i];
	//
	//	#ifdef VOXEL_DEBUG
	//	fprintf(debug, "drawing dot: %8.2f, %8.2f, %d %02X = %08X\n", dot.position[0], dot.position[1], dot.color, dot.color, ts_palette[dot.color]);
	//	fflush(debug);
	//	#endif
	//
	//	if(ts_palette[dot.color] == 0){
	//		rdPtr->pSf->SetPixel((int)dot.position[0], (int)dot.position[1], ts_palette[dot.color] + 1);
	//	} else {
	//		rdPtr->pSf->SetPixel((int)dot.position[0], (int)dot.position[1], ts_palette[dot.color]);
	//	}
	//}

	#ifdef VOXEL_DEBUG
	fflush(debug);
	fclose(debug);
	#endif
}







#endif
