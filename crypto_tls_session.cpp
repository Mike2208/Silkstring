#include "crypto_tls_session.h"
#include "error_exception.h"
#include "vector_t.h"

namespace crypto_tls_session
{
	using error_exception::Exception;
	using error_exception::error_t;
	using error_exception::ERROR_NUM;

	using vector_t::byte_vector_t;

	TLSSession::TLSSession(gnutls_init_flags_t InitFlags)
	{
		this->Initialize(InitFlags);
	}

	TLSSession::TLSSession(const TLSSession &S)
	{
		this->Copy(S);
	}

	TLSSession &TLSSession::operator=(const TLSSession &S)
	{
		this->Deinitialize();

		this->Copy(S);

		return *this;
	}

	TLSSession::TLSSession(TLSSession &&S) noexcept : _Session(std::move(S._Session))
	{
		S._Session = nullptr;
	}

	TLSSession &TLSSession::operator=(TLSSession &&S) noexcept
	{
		this->Deinitialize();

		this->_Session = std::move(S._Session);
		S._Session = nullptr;

		return *this;
	}

	TLSSession::~TLSSession() noexcept
	{
		this->Deinitialize();
	}

	TLSSession::operator gnutls_session_t() const
	{
		return this->_Session;
	}

	TLSSession::operator gnutls_session_t&()
	{
		return this->_Session;
	}

	void TLSSession::Initialize(gnutls_init_flags_t InitFlags)
	{
		if(this->_Session == nullptr)
		{
			auto err = gnutls_init(&this->_Session, InitFlags);

			if(err < 0)
				throw Exception(error_t(err), "ERROR TLSSession::Initialize(): Couldn't initialize session\n");
		}
	}

	void TLSSession::Deinitialize() noexcept
	{
		if(this->_Session != nullptr)
		{
			gnutls_deinit(this->_Session);

			this->_Session = nullptr;
		}
	}

	void TLSSession::Copy(const TLSSession &S)
	{
		this->Initialize(static_cast<gnutls_init_flags_t>(GNUTLS_SERVER));

		byte_vector_t tmpBuffer;
		size_t tmpBufferSize = 5*1024;
		tmpBuffer.resize(tmpBufferSize);

		auto err = gnutls_session_get_data(S._Session, &tmpBuffer.front(), &tmpBufferSize);
		if(err == GNUTLS_E_SHORT_MEMORY_BUFFER)
		{
			// tmpBufferSize changed to correct length via gnutls_session_get_data
			tmpBuffer.resize(tmpBufferSize);

			err = gnutls_session_get_data(S._Session, &tmpBuffer.front(), &tmpBufferSize);
		}

		if(err < 0)
			throw Exception(ERROR_NUM, "ERROR TLSSession::Copy(const TLSSession &S): Failed to get session data\n");

		tmpBuffer.resize(tmpBufferSize);

		err = gnutls_session_set_data(this->_Session, &tmpBuffer.front(), tmpBuffer.size());
		if(err < 0)
			throw Exception(ERROR_NUM, "ERROR TLSSession::Copy(const TLSSession &S): Failed to set session data\n");
	}





	class TestTLSSession
	{
		public:

			static bool Testing();
	};

	bool TestTLSSession::Testing()
	{
		try
		{
			TLSSession testSession(static_cast<gnutls_init_flags_t>(GNUTLS_SERVER));

			auto testCopy = testSession;
			(void)testCopy;

			auto testMove = std::move(testSession);
			(void)testMove;

			testCopy = testMove;

			return 1;
		}
		catch(Exception)
		{
			return 0;
		}
	}
}
