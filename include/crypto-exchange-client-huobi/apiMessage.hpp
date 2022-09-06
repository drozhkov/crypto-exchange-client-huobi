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

/// apiMessage.hpp
///
/// 0.0 - created (Denis Rozhkov <denis@rozhkoff.com>)
///

#ifndef __CRYPTO_EXCHANGE_CLIENT_HUOBI__API_MESSAGE__H
#define __CRYPTO_EXCHANGE_CLIENT_HUOBI__API_MESSAGE__H


#include <iostream>
#include <vector>

#include "boost/json.hpp"

#include "crypto-exchange-client-core/core.hpp"
#include "crypto-exchange-client-core/exception.hpp"
#include "crypto-exchange-client-core/apiMessage.hpp"


namespace as::cryptox::huobi {

	class ApiMessage : public ::as::cryptox::ApiMessage<ApiMessage> {};

	class ApiRequest : public ApiMessage {
	public:
		static as::t_string SettingsCommonSymbols()
		{
			return AS_T( "/v2/settings/common/symbols" );
		}

		static as::t_string Buy( const as::t_stringview & symbolName,
			const FixedNumber & price,
			const FixedNumber & quantity )
		{

			return ( as::t_string( AS_T( "command=buy&currencyPair=" ) ) +
				symbolName.data() + AS_T( "&rate=" ) + price.toString() +
				AS_T( "&amount=" ) + quantity.toString() );
		}

		static as::t_string Sell( const as::t_stringview & symbolName,
			const FixedNumber & price,
			const FixedNumber & quantity )
		{

			return ( as::t_string( AS_T( "command=sell&currencyPair=" ) ) +
				symbolName.data() + AS_T( "&rate=" ) + price.toString() +
				AS_T( "&amount=" ) + quantity.toString() );
		}
	};

	class ApiResponseSettingsCommonSymbols : public ApiMessage {
	public:
		struct Pair {
			as::t_string name;
			as::t_string baseName;
			as::t_string quoteName;
		};

	protected:
		std::vector<Pair> m_pairs;

	public:
		static ApiResponseSettingsCommonSymbols deserialize(
			const ::as::t_string & s )
		{

			auto v = boost::json::parse( s );
			auto & o = v.get_object();

			if ( !o.contains( "status" ) || o["status"].get_string() != "ok" ) {
				throw ::as::Exception(
					AS_T( "ApiResponseSettingsCommonSymbols" ) );
			}

			ApiResponseSettingsCommonSymbols result;

			for ( const auto & e : o["data"].get_array() ) {
				auto & s = e.get_object();

				if ( s.at( "state" ).get_string() != "online" ) {
					continue;
				}

				Pair pair;
				pair.name.assign( s.at( "sc" ).get_string() );
				pair.baseName.assign( s.at( "bc" ).get_string() );
				pair.quoteName.assign( s.at( "qc" ).get_string() );

				result.m_pairs.push_back( std::move( pair ) );
			}

			return result;
		}

		const std::vector<Pair> & Pairs() const
		{
			return m_pairs;
		}
	};

	class ApiResponseOrders : public ApiMessage {
	protected:
		::as::t_string m_orderId;

	public:
		static ApiResponseOrders deserialize( const ::as::t_string & s )
		{
			auto v = boost::json::parse( s );
			auto & o = v.get_object();

			if ( o.contains( "error" ) ) {
				throw ::as::Exception( AS_T( "ApiResponseOrders" ) );
			}

			ApiResponseOrders result;

			result.m_orderId.assign( o["orderNumber"].get_string() );

			return result;
		}

		::as::t_string & OrderId()
		{
			return m_orderId;
		}
	};

} // namespace as::cryptox::huobi


#endif
