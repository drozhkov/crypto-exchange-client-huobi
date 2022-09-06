/*
MIT License
Copyright (c) 2022 Denis Rozhkov <denis@rozhkoff.com>
Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:
The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

/// wsMessage.hpp
///
/// 0.0 - created (Denis Rozhkov <denis@rozhkoff.com>)
///

#ifndef __CRYPTO_EXCHANGE_CLIENT_HUOBI__WS_MESSAGE__H
#define __CRYPTO_EXCHANGE_CLIENT_HUOBI__WS_MESSAGE__H


#include "boost/json.hpp"

#include "crypto-exchange-client-core/core.hpp"
#include "crypto-exchange-client-core/client.hpp"
#include "crypto-exchange-client-core/wsMessage.hpp"

#include "crypto-exchange-client-huobi/apiMessage.hpp"


namespace as::cryptox::huobi {

	class WsMessage : public ::as::cryptox::WsMessage {
	public:
		static const ::as::cryptox::t_api_message_type_id TypeIdPing = 100;
		static const ::as::cryptox::t_api_message_type_id
			TypeIdPriceBookTicker = 101;

		static const ::as::cryptox::t_api_message_type_id
			TypeIdAccountNotifications = 102;

	protected:
		virtual void deserialize( boost::json::value & o ) = 0;

	public:
		WsMessage( t_api_message_type_id typeId )
			: ::as::cryptox::WsMessage( typeId )
		{
		}

		static std::shared_ptr<::as::cryptox::ApiMessageBase> deserialize(
			const char * data, size_t size, bool isV2 );

		static std::string Pong( uint64_t ts, bool isV2 )
		{
			boost::json::object o;

			if ( isV2 ) {
				o["action"] = "pong";
				o["data"] = { { "ts", ts } };
			}
			else {
				o["pong"] = ts;
			}

			return boost::json::serialize( o );
		}

		static as::t_string Subscribe(
			const as::t_string & topicName, bool isV2 )
		{

			boost::json::object o;

			if ( isV2 ) {
			}
			else {
				o["sub"] = topicName;
				o["id"] = AS_TOSTRING( ApiMessage::RequestId() );
			}

			return boost::json::serialize( o );
		}
	};

	class WsMessagePing : public WsMessage {
	protected:
		uint64_t m_ts{ 0 };

	protected:
		void deserialize( boost::json::value & o ) override;

	public:
		WsMessagePing()
			: WsMessage( TypeIdPing )
		{
		}

		uint64_t Ts() const
		{
			return m_ts;
		}
	};

	class WsMessagePingV2 : public WsMessagePing {
	protected:
		void deserialize( boost::json::value & o ) override;
	};

	class WsMessagePriceBookTicker : public WsMessage {
	protected:
		as::t_string m_symbolName;
		::as::FixedNumber m_askPrice;
		::as::FixedNumber m_askSize;
		::as::FixedNumber m_bidPrice;
		::as::FixedNumber m_bidSize;

	protected:
		void deserialize( boost::json::value & o ) override;

	public:
		WsMessagePriceBookTicker()
			: WsMessage( TypeIdPriceBookTicker )
		{
		}

		const as::t_string & SymbolName() const
		{
			return m_symbolName;
		}

		::as::FixedNumber & AskPrice()
		{
			return m_askPrice;
		}

		::as::FixedNumber & AskSize()
		{
			return m_askSize;
		}

		::as::FixedNumber & BidPrice()
		{
			return m_bidPrice;
		}

		::as::FixedNumber & BidSize()
		{
			return m_bidSize;
		}
	};

	class WsMessageAccountNotifications : public WsMessage {
	public:
		struct Notification {};


	protected:
	protected:
		void deserialize( boost::json::value & o ) override;

	public:
		WsMessageAccountNotifications()
			: WsMessage( TypeIdAccountNotifications )
		{
		}
	};

} // namespace as::cryptox::huobi


#endif
