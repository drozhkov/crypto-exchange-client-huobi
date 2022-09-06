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

/// client.cpp
///
/// 0.0 - created (Denis Rozhkov <denis@rozhkoff.com>)
///

#include <sstream>

#include "boost/json.hpp"
#include "boost/iostreams/device/array.hpp"
#include "boost/iostreams/filtering_stream.hpp"
#include "boost/iostreams/filter/gzip.hpp"
#include "boost/iostreams/copy.hpp"

#include "crypto-exchange-client-core/logger.hpp"
#include "crypto-exchange-client-core/exception.hpp"

#include "crypto-exchange-client-huobi/client.hpp"
#include "crypto-exchange-client-huobi/wsMessage.hpp"


namespace as::cryptox::huobi {

	void Client::addAuthHeaders(
		HttpHeaderList & headers, ::as::t_string & body )
	{

		auto nonce = UnixTs<std::chrono::milliseconds>();

		body += ( body.empty() ? AS_T( "" ) : AS_T( "&" ) ) +
			as::t_string( AS_T( "nonce=" ) ) + AS_TOSTRING( nonce );

		auto sign = hmacSha512( m_apiSecret, body );
		auto signHex = toHex( as::t_buffer( sign.data(), sign.size() ) );

		headers.add( AS_T( "Key" ), m_apiKey );
		headers.add( AS_T( "Sign" ), signHex );

		headers.add( AS_T( "Content-Type" ),
			AS_T( "application/x-www-form-urlencoded " ) );
	}

	void Client::wsErrorHandler(
		WsClient & client, int code, const as::t_string & message )
	{
	}

	void Client::wsHandshakeHandler( WsClient & client )
	{
		AS_CALL( m_clientReadyHandler, *this, client.Index() );
		client.readAsync();
	}

	bool Client::wsReadHandler(
		WsClient & client, const char * data, size_t size )
	{

		try {
			std::string dataS;

			// holy shit!!! instead of the plain transport-level deflate they
			// use gzip...
			if ( client.Index() == WsClientApiIndex ||
				client.Index() == WsClientApiFeedIndex ) {

				boost::iostreams::filtering_istream fis;
				fis.push( boost::iostreams::gzip_decompressor() );
				fis.push( boost::iostreams::array_source( data, data + size ) );

				std::stringstream ss;
				boost::iostreams::copy( fis, ss );
				dataS = ss.str();
				data = dataS.c_str();
				size = dataS.length();
			}

			AS_LOG_TRACE_LINE(
				client.Index() << ": " << std::string( data, size ) );

			auto message = WsMessage::deserialize(
				data, size, client.Index() == WsClientApiV2Index );

			switch ( message->TypeId() ) {
				case WsMessage::TypeIdPing: {
					auto m = static_cast<WsMessagePing *>( message.get() );
					auto s = WsMessage::Pong(
						m->Ts(), client.Index() == WsClientApiV2Index );

					std::cout << s << std::endl;

					client.writeAsync( s.c_str(), s.length() );
				}

				break;

				case WsMessage::TypeIdPriceBookTicker: {
					auto m = static_cast<WsMessagePriceBookTicker *>(
						message.get() );

					as::cryptox::t_price_book_ticker t;
					t.symbol = toSymbol( m->SymbolName().c_str() );
					t.askPrice = std::move( m->AskPrice() );
					t.askQuantity = std::move( m->AskSize() );
					t.bidPrice = std::move( m->BidPrice() );
					t.bidQuantity = std::move( m->BidSize() );

					callSymbolHandler( t.symbol,
						m_priceBookTickerHandlerMap,
						client.Index(),
						t );
				}

				break;

					//	case WsMessage::TypeIdAccountNotifications: {
					//		auto m = static_cast<WsMessageAccountNotifications
					//*>( 			message.get() );

					//		// as::cryptox::t_order_update u;
					//		// u.orderId = std::move( m->OrderId() );

					//		// AS_CALL( m_orderUpdateHandler, *this, u );
					//	}

					//	break;
			}
		}
		catch ( const std::exception & x ) {
			AS_LOG_ERROR_LINE( x.what() );
		}
		catch ( ... ) {
		}

		return true;
	}

	void Client::initSymbolMap()
	{
		AS_LOG_INFO_LINE( "initializing..." );

		as::cryptox::Client::initSymbolMap();

		auto apiRes = apiReqSettingsCommonSymbols();
		m_pairList.resize( apiRes.Pairs().size() + 2 );
		m_pairList[0] = as::cryptox::Pair( as::cryptox::Coin::_undef,
			as::cryptox::Coin::_undef,
			AS_T( "undefined" ) );

		size_t index = 1;

		for ( const auto & p : apiRes.Pairs() ) {
			AS_LOG_TRACE_LINE( p.name );

			as::cryptox::Coin quote = toCoin( p.quoteName.c_str() );
			as::cryptox::Coin base = toCoin( p.baseName.c_str() );

			as::cryptox::Pair pair( base, quote, p.name );

			m_pairList[index] = pair;

			addSymbolMapEntry(
				p.name, static_cast<as::cryptox::Symbol>( index ) );

			index++;
		}

		AS_LOG_INFO_LINE( "done" );
	}

	void Client::initWsClient( size_t index )
	{
		as::cryptox::Client::initWsClient( index );
	}

	ApiResponseSettingsCommonSymbols Client::apiReqSettingsCommonSymbols()
	{
		auto url = m_httpApiUrls[HttpClientApiIndex].add(
			ApiRequest::SettingsCommonSymbols() );

		auto res = m_httpClient.get( url, HttpHeaderList() );

		return ApiResponseSettingsCommonSymbols::deserialize( res );
	}

	bool Client::subscribe( size_t index, const as::t_string & topicName )
	{
		auto buffer =
			WsMessage::Subscribe( topicName, WsClientApiV2Index == index );

		auto r = callWsClient<bool>( index, [&buffer]( WsClient * client ) {
			client->write( buffer.c_str(), buffer.length() );
			return true;
		} );

		return ( r.first && r.second );
	}

	void Client::run( const t_exchangeClientReadyHandler & handler,
		const std::function<void( size_t )> & beforeRun )
	{

		as::cryptox::Client::run( handler );
	}

	bool Client::subscribePriceBookTicker( size_t wsClientIndex,
		as::cryptox::Symbol symbol,
		const t_priceBookTickerHandler & handler )
	{

		if ( WsClientApiIndex != wsClientIndex ) {
			return false;
		}

		as::cryptox::Client::subscribePriceBookTicker(
			wsClientIndex, symbol, handler );

		auto topicName = as::t_string( AS_T( "market." ) ) + toName( symbol ) +
			AS_T( ".bbo" );

		AS_LOG_TRACE_LINE( topicName );

		return subscribe( wsClientIndex, topicName );
	}

	void Client::subscribeOrderUpdate( const t_orderUpdateHandler & handler )
	{
		as::cryptox::Client::subscribeOrderUpdate( handler );
	}

	t_order Client::placeOrder( Direction direction,
		as::cryptox::Symbol symbol,
		const FixedNumber & price,
		const FixedNumber & quantity )
	{

		return ( t_order() );
	}

} // namespace as::cryptox::huobi
