
#ifndef CHEAT_H
#define CHEAT_H

#include <string>
#include <vector>
#include <nds.h>

void writeCheatsToFile(std::string data, const char* path);

class CheatCodelist
{
public:
	CheatCodelist (void)
	{
	}
	
	~CheatCodelist ();

	bool parse(const std::string& aFileName);

	bool searchCheatData(FILE* aDat,u32 gamecode,u32 crc32,long& aPos,size_t& aSize);

	bool parseInternal(FILE* aDat,u32 gamecode,u32 crc32);

	void generateList(void);

	bool romData(const std::string& aFileName,u32& aGameCode,u32& aCrc32);

  void selectCheats(std::string filename);

  void onGenerate(void);

	private:
    struct sDatIndex
    {
      u32 _gameCode;
      u32 _crc32;
      u64 _offset;
    };
    class cParsedItem
    {
      public:
        std::string _title;
        std::string _comment;
        std::string _cheat;
        u32 _flags;
        u32 _offset;
        cParsedItem(const std::string& title,const std::string& comment,u32 flags,u32 offset=0):_title(title),_comment(comment),_flags(flags),_offset(offset) {};
        enum
        {
          EFolder=1,
          EInFolder=2,
          EOne=4,
          ESelected=8,
          EOpen=16
        };
    };
  private:
    std::vector<cParsedItem> _data;
    std::vector<size_t> _indexes;
  public:
    std::string getCheats();
	
private:
	enum TOKEN_TYPE {TOKEN_DATA, TOKEN_TAG_START, TOKEN_TAG_END, TOKEN_TAG_SINGLE};

	std::string nextToken (FILE* fp, TOKEN_TYPE& tokenType);

} ;

#endif // CHEAT_H

