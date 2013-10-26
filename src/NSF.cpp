#include "NSF.h"

NSF::NSF(void)
{
	song_count = 0;
	song_start = 0;

	load_address = 0;
	init_address = 0;
	play_address = 0;

	speed_ntsc = 0;
	speed_pal = 0;

	pal_ntsc_bits = 0;
	extra_soundchips = 0;

	data = NULL;
	size = 0;

	Log->Debug("NSF created");
}

NSF::~NSF(void)
{
	Log->Debug("NSF destroyed");
}
int NSF::Load( const char* name )
{
	FILE* rom = fopen(name, "rb");

	if (!rom)
	{
		Log->Error("NSF.cpp: Cannot open %s", rom);
		return 1;
	}

	fseek( rom, 0, SEEK_END );
	size = ftell( rom );
	rewind( rom );
	
	char magic[5];
	char buffer[32];
	fread( magic, 1, 5, rom );
	if ( memcmp( magic, "NESM\x1A", 5 ) )
	{
		Log->Error("NSF.cpp: Wrong magic: expected NESM\\x1a");
		return 2; // Wrong MAGIC
	}
	uint8_t version = fgetc( rom );
	if (version != 0x01)
	{
		Log->Error("NSF.cpp: Wrong version: expected 0x01");
		return 3; 
	}

	size -= 0x80;

	song_count = fgetc( rom );
	song_start = fgetc( rom );

	load_address = fgetc( rom ) | fgetc( rom ) << 8;
	init_address = fgetc( rom ) | fgetc( rom ) << 8;
	play_address = fgetc( rom ) | fgetc( rom ) << 8;

	fread( buffer, 1, 32, rom );
	Log->Info("Name: %s", buffer);
	
	fread( buffer, 1, 32, rom );
	Log->Info("Artist: %s", buffer);
	
	fread( buffer, 1, 32, rom );
	Log->Info("Copyright: %s", buffer);

	speed_ntsc = fgetc( rom ) | fgetc( rom ) << 8;

	for (int i = 0; i<8; i++)
	{
		if (fgetc( rom ) != 0) 
		{
			Log->Error("NSF.cpp: Banking used, not supported");
			return 4;
		}
	}

	speed_pal = fgetc( rom ) | fgetc( rom ) << 8;
	pal_ntsc_bits = fgetc( rom );
	extra_soundchips = fgetc( rom );

	for (int i = 0; i<4; i++)
	{
		if (fgetc( rom ) != 0) 
		{
			Log->Error("NSF.cpp: Extra bytes: expected 0x00");
			return 5;
		}
	}

	data = (uint8_t*)malloc( size );
	if ( !data )
	{
		Log->Error("NSF.cpp: Cannot malloc");
		return 6;
	}
	fread( data, 1, size, rom );

	fclose( rom );
	Log->Debug("iNES loaded");
	return 0;
}
