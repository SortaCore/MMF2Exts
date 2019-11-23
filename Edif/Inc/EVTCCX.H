
#ifndef _EvtCCX_h
#define _EvtCCX_h

#ifndef IN_EXT_VERSION2

// Maximum number of parameters
#define EVI_MAXPARAMS				16

// Structure de definition des conditions / actions POUR LES EXTENSIONS V1
#ifdef __cplusplus
class infosEvents { 
public:
#else
typedef struct infosEvents {
#endif
	short 	code;					// Le numero de type + code event
	short	flags;					// Les flags a mettre dans l'event
	short	param[2];				// Le type des parametres
	short	paramTitle[2];			// Le titre de chacun des params
#ifdef __cplusplus
};
#else
} infosEvents;
#endif
typedef	infosEvents FAR *			LPINFOEVENTS;
typedef	infosEvents *				PINFOEVENTS;

// Event information structure
// ---------------------------
typedef struct {
	short		menu;					// Menu identifier
	short		string;					// String identifier
	infosEvents	infos;					// Sub structure defined in CNPF.H
	} eventInformations;
typedef	eventInformations _far *		LPEVENTINFOS;
typedef	eventInformations *				NPEVENTINFOS;

#endif


// Structure V2 : nombre de parametres variable...
typedef struct infosEventsV2 {
	short 	code;					// Le numero de type + code event
	short	flags;					// Les flags a mettre dans l'event
	short	nParams;				// Le nombre de parametres
//	short	param[X];				// Le type des parametres
//	short	paramTitle[X];			// Le titre de chacun des params
} infosEventsV2;
typedef	infosEventsV2 FAR *			LPINFOEVENTSV2;


// Event information structure
typedef struct {
	short			menu;			// Menu identifier
	short			string;			// String identifier
	infosEventsV2	infos;			// Sub structure
} eventInformations2;
typedef	eventInformations2 FAR *	LPEVENTINFOS2;

#define EVINFO2_NEXT(p)			((LPEVENTINFOS2)((LPBYTE)p + sizeof(eventInformations2) + p->infos.nParams * 2 * sizeof(short)))
#define	EVINFO2_PARAM(p,n)		(*(LPWORD)((LPBYTE)p + sizeof(eventInformations2) + n * sizeof(short)))
#define	EVINFO2_PARAMTITLE(p,n)	(*(LPWORD)((LPBYTE)p + sizeof(eventInformations2) + p->infos.nParams * sizeof(short) + n * sizeof(short)))

#endif // _EvtCCX_h
