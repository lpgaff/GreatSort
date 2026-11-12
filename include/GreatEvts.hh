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

class GreatDoubleSidedDetectorEvt : public TObject {

public:

	// setup functions
	GreatDoubleSidedDetectorEvt() {};
	~GreatDoubleSidedDetectorEvt() {};

	GreatDoubleSidedDetectorEvt( const GreatDoubleSidedDetectorEvt &in ) {
		SetEvent(in);
	};

	void SetEvent( const GreatDoubleSidedDetectorEvt &in ){
		ntime	= in.GetNTime();
		ptime	= in.GetPTime();
		nenergy	= in.GetNEnergy();
		penergy	= in.GetPEnergy();
		id		= in.GetID();
		nid		= in.GetNStrip();
		pid		= in.GetPStrip();
		type	= in.GetType();
	};

	inline void SetNTime( double t ){ ntime = t; };
	inline void SetPTime( double t ){ ptime = t; };
	inline void SetNEnergy( float e ){ nenergy = e; };
	inline void SetPEnergy( float e ){ penergy = e; };
	inline void SetID( unsigned short i ){ id = i; };
	inline void SetNStrip( unsigned short i ){ nid = i; };
	inline void SetPStrip( unsigned short i ){ pid = i; };
	inline void SetType( unsigned char t ){ type = t; };

	inline double			GetTime() const { return ptime; };
	inline double			GetNTime() const { return ntime; };
	inline double			GetPTime() const { return ptime; };
	inline float			GetEnergy() const { return penergy; };
	inline float			GetNEnergy() const { return nenergy; };
	inline float			GetPEnergy() const { return penergy; };
	inline unsigned short	GetID() const { return id; };
	inline unsigned short	GetNStrip() const { return nid; };
	inline unsigned short	GetPStrip() const { return pid; };
	inline unsigned char	GetType() const { return type; };


protected:

	double			ntime = 0;		///< time stamp of the event in the n-side
	double			ptime = 0;		///< time stamp of the event in the p-side
	float			nenergy = 0;	///< Energy in the n-side of the detector
	float			penergy = 0;	///< Energy in the p-side of the detector
	unsigned short	id = 32767;		///< Detector ID
	unsigned short	nid = 32767;	///< n-side strip
	unsigned short	pid = 32767;	///< p-side stirp
	unsigned short	seg = 32767;	///< Detector segment
	unsigned char	type = 255;		///< Detector type:
									///< 0 - CeBr
									///< 1 - HPGe(?), segments, etc?!??!??!
									///< 2 - TAC

	ClassDef( GreatDoubleSidedDetectorEvt, 2 );

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

class GreatDSSSDEvt : public GreatDoubleSidedDetectorEvt {

public:


protected:

	ClassDef( GreatDSSSDEvt, 1 );

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
	void AddEvt( std::shared_ptr<GreatDSSSDEvt> event ){
		GreatDSSSDEvt fill_evt( *(event.get()) );
		dsssd_event.push_back( fill_evt );
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
	inline unsigned int GetDSSSDMultiplicity() const {
		return dsssd_event.size();
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
	inline std::shared_ptr<GreatDSSSDEvt> GetDSSSDEvt( unsigned int i ) const {
		if( i < dsssd_event.size() ) return std::make_shared<GreatDSSSDEvt>( dsssd_event.at(i) );
		else return nullptr;
	};

	void ClearEvt(){
		std::vector<GreatTACEvt>().swap(tac_event);
		std::vector<GreatCeBr3Evt>().swap(cebr3_event);
		std::vector<GreatHPGeEvt>().swap(hpge_event);
		std::vector<GreatDSSSDEvt>().swap(dsssd_event);
	};
	double GetTime() const;

	
protected:

	std::vector<GreatTACEvt> tac_event;
	std::vector<GreatCeBr3Evt> cebr3_event;
	std::vector<GreatHPGeEvt> hpge_event;
	std::vector<GreatDSSSDEvt> dsssd_event;

	ClassDef( GreatEvts, 4 )

};

#endif

