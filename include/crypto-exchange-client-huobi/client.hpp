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

/// client.hpp
///
/// 0.0 - created (Denis Rozhkov <denis@rozhkoff.com>)
///

#ifndef __CRYPTO_EXCHANGE_CLIENT_HUOBI__CLIENT__H
#define __CRYPTO_EXCHANGE_CLIENT_HUOBI__CLIENT__H


#include "crypto-exchange-client-core/httpClient.hpp"
#include "crypto-exchange-client-core/client.hpp"

#include "crypto-exchange-client-huobi/apiMessage.hpp"


namespace as::cryptox::huobi {

	class Client : public as::cryptox::Client {
	public:
		static const size_t HttpClientApiIndex = 0;
		static const size_t WsClientApiIndex = 0;
		static const size_t WsClientApiFeedIndex = 1;
		static const size_t WsClientApiV2Index = 2;

	protected:
		as::t_string m_apiKey;
		as::t_string m_apiSecret;

	private:
		void addAuthHeaders( HttpHeaderList & headers, as::t_string & body );

	protected:
		void wsErrorHandler(
			as::WsClient &, int, const as::t_string & ) override;

		void wsHandshakeHandler( as::WsClient & ) override;
		bool wsReadHandler( as::WsClient &, const char *, size_t ) override;

		void initCoinMap() override
		{
			cryptox::Client::initCoinMap();

			addCoinMapEntry( AS_T( "btc" ), Coin::BTC );
			addCoinMapEntry( AS_T( "eth" ), Coin::ETH );
			addCoinMapEntry( AS_T( "kcs" ), Coin::KCS );
			addCoinMapEntry( AS_T( "trx" ), Coin::TRX );
			addCoinMapEntry( AS_T( "usdt" ), Coin::USDT );
		}

		void initSymbolMap() override;
		void initWsClient( size_t index ) override;

		bool subscribe( size_t wsClientIndex, const as::t_string & topicName );

	public:
		Client( const as::t_string & apiKey = AS_T( "" ),
			const as::t_string & apiSecret = AS_T( "" ),
			const as::t_string & httpApiUrl = AS_T(
				"https://api-aws.huobi.pro" ),
			const as::t_string & wsApiUrl = AS_T( "wss://api.huobi.pro/ws" ),
			const as::t_string & wsApiFeedUrl = AS_T(
				"wss://api.huobi.pro/feed" ),
			const as::t_string & wsApiV2Url = AS_T(
				"wss://api.huobi.pro/ws/v2" ) )
			: as::cryptox::Client(
				  { httpApiUrl }, { wsApiUrl, wsApiFeedUrl, wsApiV2Url } )
			, m_apiKey( apiKey )
			, m_apiSecret( apiSecret )
		{
		}

		ApiResponseSettingsCommonSymbols apiReqSettingsCommonSymbols();

		void run(
			const t_exchangeClientReadyHandler & handler,
			const std::function<void( size_t )> & beforeRun = []( size_t ) {
			} ) override;

		bool subscribePriceBookTicker( size_t wsClientIndex,
			as::cryptox::Symbol symbol,
			const t_priceBookTickerHandler & handler ) override;

		void subscribeOrderUpdate(
			const t_orderUpdateHandler & handler ) override;

		t_order placeOrder( Direction direction,
			as::cryptox::Symbol symbol,
			const FixedNumber & price,
			const FixedNumber & quantity ) override;
	};

} // namespace as::cryptox::huobi


#endif
