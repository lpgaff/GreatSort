#ifndef __DATAPACKETS_HH
#define __DATAPACKETS_HH

#include <memory>
#include <iostream>

#include "TObject.h"

class GreatCaenData : public TObject {
	
public:

	GreatCaenData();
	~GreatCaenData();

	inline double			GetTime() { return (double)timestamp + finetime; };
	inline unsigned long	GetTimeStamp() { return timestamp; };
	inline float			GetFineTime() { return finetime; };
	inline float			GetBaseline() { return baseline; };
	inline unsigned short	GetTraceLength() { return trace.size(); };
	inline std::vector<unsigned short> GetTrace() { return trace; };
	inline unsigned short	GetSample( unsigned int i = 0 ) {
		if( i >= trace.size() ) return 0;
		return trace.at(i);
	};
	inline unsigned char	GetModule() { return mod; };
	inline unsigned char	GetChannel() { return ch; };
	inline unsigned short	GetCharge() { return Qlong; };
	inline unsigned short	GetQlong() { return Qlong; };
	inline unsigned short	GetQshort() { return Qshort; };
	inline unsigned short	GetQdiff() { return (int)Qlong-(int)Qshort; };
	inline float			GetEnergy() { return energy; };
	inline bool				IsOverThreshold() { return thres; };

	inline void	SetTimeStamp( unsigned long long t ) { timestamp = t; };
	inline void	SetFineTime( float t ) { finetime = t; };
	inline void	SetBaseline( float b ) { baseline = b; };
	inline void	SetTrace( std::vector<unsigned short> t ) { trace = t; };
	inline void AddSample( unsigned short s ) { trace.push_back(s); };
	inline void	SetQlong( unsigned short q ) { Qlong = q; };
	inline void	SetQshort( unsigned short q ) { Qshort = q; };
	inline void	SetModule( unsigned char m ) { mod = m; };
	inline void	SetChannel( unsigned char c ) { ch = c; };
	inline void SetEnergy( float e ){ energy = e; };
	inline void SetThreshold( bool t ){ thres = t; };

	inline void ClearTrace() { trace.clear(); };
	void ClearData();

protected:
	
	unsigned long long			timestamp;
	float						finetime;
	float						baseline;
	std::vector<unsigned short>	trace;
	unsigned short				Qlong;
	unsigned short				Qshort;
	unsigned char				mod;
	unsigned char				ch;
	bool						thres;		///< is the energy over threshold?
	float						energy;

	
	ClassDef( GreatCaenData, 6 )
	
};

class GreatInfoData : public TObject {
	
public:

	GreatInfoData();
	~GreatInfoData();
	
	inline double	 			GetTime(){ return (double)timestamp; };
	inline unsigned long long	GetTimeStamp(){ return timestamp; };
	inline unsigned char 		GetCode(){ return code; };
	inline unsigned char 		GetModule(){ return mod; };
	
	inline void SetTimeStamp( unsigned long long t ){ timestamp = t; };
	inline void SetCode( unsigned char c ){ code = c; };
	inline void SetModule( unsigned char m ){ mod = m; };

	void ClearData();

protected:
	
	unsigned long long		timestamp;	///< timestamp of info event
	unsigned char			code;	///< code here represents which information timestamp we have
	unsigned char			mod;	///< module ID of the event
	/// code = 4 is extended timestimp, i.e. next 16 bits

	
	ClassDef( GreatInfoData, 2 )
	
};

class GreatDataPackets : public TObject {
	
public:
	
	GreatDataPackets() {};
	~GreatDataPackets() {};

	GreatDataPackets( std::shared_ptr<GreatDataPackets> in ) { SetData(in); };
	GreatDataPackets( std::shared_ptr<GreatCaenData> in ){ SetData(in); };
	GreatDataPackets( std::shared_ptr<GreatInfoData> in ){ SetData(in); };

	inline bool	IsCaen() const { return caen_packets.size(); };
	inline bool	IsInfo() const { return info_packets.size(); };

	void SetData( std::shared_ptr<GreatDataPackets> in ){
		if( in->IsCaen() ) SetData( in->GetCaenData() );
		if( in->IsInfo() ) SetData( in->GetInfoData() );
	};
	void SetData( std::shared_ptr<GreatCaenData> data );
	void SetData( std::shared_ptr<GreatInfoData> data );

	// These methods are not very safe for access
	inline std::shared_ptr<GreatCaenData> GetCaenData() const {
		return std::make_shared<GreatCaenData>( caen_packets.at(0) );
	};
	inline std::shared_ptr<GreatInfoData> GetInfoData() const {
		return std::make_shared<GreatInfoData>( info_packets.at(0) );
	};

	// Complicated way to get the time...
	double GetTime() const;
	unsigned long long GetTimeStamp() const;
	UInt_t GetTimeMSB() const;
	UInt_t GetTimeLSB() const;

	// Sorting function to do time ordering
	bool operator <( const GreatDataPackets &rhs ) const {
		return( GetTime() < rhs.GetTime() );
	};


	void ClearData();

protected:
	
	std::vector<GreatCaenData> caen_packets;
	std::vector<GreatInfoData> info_packets;

	ClassDef( GreatDataPackets, 2 )

};


#endif
