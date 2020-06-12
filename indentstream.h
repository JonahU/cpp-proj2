#ifndef INDENT_STREAM_H
#  define INDENT_STREAM_H

#include <streambuf>
#include <ostream>

namespace mpcs {

class IndentStreamBuf : public std::streambuf
{
public:
    IndentStreamBuf(std::ostream &stream)
        : wrappedStreambuf(stream.rdbuf()), isLineStart(true), myIndent(0) {}
	virtual int overflow(int outputVal) override
	{
		if (outputVal == traits_type::eof())
			return traits_type::eof();
        if(outputVal == '\n') {
            isLineStart = true;
        } else if(isLineStart) {
            for(size_t i = 0; i < myIndent; i++) {
               wrappedStreambuf->sputc(' ');
            }
            isLineStart = false;
        }
        wrappedStreambuf->sputc(static_cast<char>(outputVal));
		return outputVal;
	}
protected:
    std::streambuf *wrappedStreambuf;
    bool isLineStart;
public:
    size_t myIndent;
};

class IndentStream : public std::ostream
{
public:
    IndentStream(std::ostream &wrappedStream)
      : std::ostream(new IndentStreamBuf(wrappedStream)) {
    }
    ~IndentStream() { delete this->rdbuf(); }
};


inline std::ostream &indent(std::ostream &ostr)
{
    IndentStreamBuf *out = dynamic_cast<IndentStreamBuf *>(ostr.rdbuf());
	if (nullptr != out) {
        out->myIndent += 4;
    }
    return ostr;
}

inline std::ostream &unindent(std::ostream &ostr)
{
    IndentStreamBuf *out = dynamic_cast<IndentStreamBuf *>(ostr.rdbuf());
    out->myIndent -= 4;
    return ostr;
}

}
#endif
