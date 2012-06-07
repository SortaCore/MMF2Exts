// RUNDATA struct automator (rSDK)
// Jamie McLaughlin, 2007

headerObject rHo;

#ifdef M_OEFLAG_MOVEMENTS
	rCom rc;
	rMvt rm;
	#ifdef M_OEFLAG_ANIMATIONS
		rAni ra;
	#endif

	#ifdef M_OEFLAG_SPRITES
		rSpr rs;
	#endif
#else
	#ifdef M_OEFLAG_ANIMATIONS
		rCom rc;
		rAni ra;
	#else
		#ifdef M_OEFLAG_SPRITE
			rCom rc;
			rSpr rs;
		#endif
	#endif
#endif
#ifdef M_OEFLAG_VALUES
	rVal rv;
#endif

LPRRDATA rRd;