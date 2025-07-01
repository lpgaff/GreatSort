#ifndef __GREATEVTS_HH
#define __GREATEVTS_HH

#include <iostream>
#include <vector>
#include <string>
#include <memory>

#include "TVector2.h"
#include "TVector3.h"
#include "TObject.h"

class GreatDetectorEvt : public TObject {

public:

	// setup functions
	GreatDetectorEvt() {};
	~GreatDetectorEvt() {};

	GreatDetectorEvt( const GreatDetectorEvt &in ) {
		SetEvent(in);
	};

	void SetEvent( const GreatDetectorEvt &in ){
		time	= in.GetTime();
		energy	= in.GetEnergy();
		id		= in.GetID();
		seg		= in.GetSegment();
		type	= in.GetType();
	};

	inline void SetTime( double t ){ time = t; };
	inline void SetEnergy( float e ){ energy = e; };
	inline void SetID( unsigned short i ){ id = i; };
	inline void SetSegment( unsigned short t ){ type = t; };
	inline void SetType( unsigned char t ){ type = t; };

	inline double			GetTime() const { return time; };
	inline float			GetEnergy() const { return energy; };
	inline unsigned short	GetID() const { return id; };
	inline unsigned short	GetSegment() const { return seg; };
	inline unsigned char	GetType() const { return type; };


protected:

	double			time = 0;		///< time stamp of the event
	float			energy = 0;		///< Energy in the detector
	unsigned short	id = 32767;		///< Detector ID
	unsigned short	seg = 32767;	///< Detector segment
	unsigned char	type = 255;		///< Detector type:
									///< 0 - CeBr
									///< 1 - HPGe(?), segments, etc?!??!??!
									///< 2 - TAC

	ClassDef( GreatDetectorEvt, 2 );

};

class GreatTACEvt : public GreatDetectorEvt {

public:

	// setup functions
	inline void		SetTACTime( float t ){ energy = t; };
	inline float	GetTACTime(){ return energy; };


protected:

	ClassDef( GreatTACEvt, 2 );

};

class GreatGammaRayEvt : public GreatDetectorEvt {

public:


protected:

	ClassDef( GreatGammaRayEvt, 2 );

};

class GreatCeBr3Evt : public GreatGammaRayEvt {

public:


protected:

	ClassDef( GreatCeBr3Evt, 1 );

};

class GreatHPGeEvt : public GreatGammaRayEvt {

public:


protected:

	ClassDef( GreatHPGeEvt, 1 );

};


class GreatEvts : public TObject {

public:
	
	// setup functions
	GreatEvts() {};
	~GreatEvts() {};

	// Adding different event types
	void AddEvt( std::shared_ptr<GreatTACEvt> event ){
		GreatTACEvt fill_evt( *(event.get()) );
		tac_event.push_back( fill_evt );
	};
	void AddEvt( std::shared_ptr<GreatCeBr3Evt> event ){
		GreatCeBr3Evt fill_evt( *(event.get()) );
		cebr3_event.push_back( fill_evt );
	};
	void AddEvt( std::shared_ptr<GreatHPGeEvt> event ){
		GreatHPGeEvt fill_evt( *(event.get()) );
		hpge_event.push_back( fill_evt );
	};

	inline unsigned int GetTACMultiplicity() const { return tac_event.size(); };
	inline unsigned int GetGammaRayMultiplicity() const {
		return cebr3_event.size() + hpge_event.size();
	};
	inline unsigned int GetCeBr3Multiplicity() const {
		return cebr3_event.size();
	};
	inline unsigned int GetHPGeMultiplicity() const {
		return hpge_event.size();
	};

	inline std::shared_ptr<GreatTACEvt> GetTACEvt( unsigned int i ) const {
		if( i < tac_event.size() ) return std::make_shared<GreatTACEvt>( tac_event.at(i) );
		else return nullptr;
	};
	inline std::shared_ptr<GreatCeBr3Evt> GetCeBr3Evt( unsigned int i ) const {
		if( i < cebr3_event.size() ) return std::make_shared<GreatCeBr3Evt>( cebr3_event.at(i) );
		else return nullptr;
	};
	inline std::shared_ptr<GreatHPGeEvt> GetHPGeEvt( unsigned int i ) const {
		if( i < hpge_event.size() ) return std::make_shared<GreatHPGeEvt>( hpge_event.at(i) );
		else return nullptr;
	};

	void ClearEvt(){
		std::vector<GreatTACEvt>().swap(tac_event);
		std::vector<GreatCeBr3Evt>().swap(cebr3_event);
		std::vector<GreatHPGeEvt>().swap(hpge_event);
	};
	double GetTime() const;

	
protected:

	std::vector<GreatTACEvt> tac_event;
	std::vector<GreatCeBr3Evt> cebr3_event;
	std::vector<GreatHPGeEvt> hpge_event;

	ClassDef( GreatEvts, 3 )

};

#endif

