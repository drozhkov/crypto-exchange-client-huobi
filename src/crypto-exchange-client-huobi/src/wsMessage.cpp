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

/// wsMessage.cpp
///
/// 0.0 - created (Denis Rozhkov <denis@rozhkoff.com>)
///

#include "crypto-exchange-client-huobi/wsMessage.hpp"


namespace as::cryptox::huobi {

	std::shared_ptr<::as::cryptox::ApiMessageBase> WsMessage::deserialize(
		const char * data, size_t size, bool isV2 )
	{

		auto v = boost::json::parse( { data, size } );
		auto & v1 = v;

		WsMessage * r = nullptr;

		if ( isV2 ) {
			auto & o = v.get_object();
			auto action = o["action"].get_string();

			if ( "ping" == action ) {
				r = new WsMessagePingV2;
				v1 = o["data"];
			}
			else if ( "req" == action ) {
				auto & ch = o["ch"].get_string();

				if ( "auth" == ch ) {
					r = new WsMessageAuthResponse;
				}
			}
		}
		else {
			auto & o = v.get_object();

			if ( o.contains( "ping" ) ) {
				r = new WsMessagePing;
			}
			else if ( o.contains( "ch" ) ) {
				auto & ch = o["ch"].get_string();

				if ( ch.starts_with( "market." ) && ch.ends_with( ".bbo" ) ) {
					r = new WsMessagePriceBookTicker;
				}
			}
		}

		if ( nullptr == r ) {
			return s_unknown;
		}

		r->deserialize( v1 );

		return std::shared_ptr<::as::cryptox::WsMessage>( r );
	}

	////

	void WsMessagePing::deserialize( boost::json::value & o )
	{
		m_ts = o.get_object()["ping"].get_int64();
	}

	////

	void WsMessagePingV2::deserialize( boost::json::value & o )
	{
		m_ts = o.get_object()["ts"].get_int64();
	}

	////

	void WsMessagePriceBookTicker::deserialize( boost::json::value & v )
	{
		auto & o = v.get_object()["tick"].get_object();
		m_askPrice.Value( o["ask"].get_double() );
		m_askSize.Value( o["askSize"].get_double() );
		m_bidPrice.Value( o["bid"].get_double() );
		m_bidSize.Value( o["bidSize"].get_double() );
		m_symbolName.assign( o["symbol"].get_string() );
	}

	////

	void WsMessageAccountNotifications::deserialize( boost::json::value & o )
	{
	}

	////

	void WsMessageAuthResponse::deserialize( boost::json::value & v )
	{
		auto & o = v.get_object();
		m_isOk = o["code"].get_int64() == 200;
	}

} // namespace as::cryptox::huobi
