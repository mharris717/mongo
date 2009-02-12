// perftest.cpp : Run db performance tests.
//

/**
 *    Copyright (C) 2009 10gen Inc.
 *
 *    This program is free software: you can redistribute it and/or  modify
 *    it under the terms of the GNU Affero General Public License, version 3,
 *    as published by the Free Software Foundation.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU Affero General Public License for more details.
 *
 *    You should have received a copy of the GNU Affero General Public License
 *    along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "stdafx.h"

#include "../../client/dbclient.h"
#include "../../db/instance.h"
#include "../../db/query.h"

#include <unittest/Registry.hpp>
#include <unittest/UnitTest.hpp>

namespace mongo {
    extern const char* dbpath;
} // namespace mongo

// Very useful function, hacky way of getting at at.
namespace UnitTest { namespace Private {
    extern std::string demangledName(const std::type_info &typeinfo);
} }

using namespace mongo;

DBClientBase *client_;

// Each test runs with a separate db, so no test does any of the startup
// (ie allocation) work for another test.
template< class T >
string testDb( T *t = 0 ) {
    string name = UnitTest::Private::demangledName( typeid( T ) );
    // Make filesystem safe.
    for( string::iterator i = name.begin(); i != name.end(); ++i )
        if ( *i == ':' )
            *i = '_';
    return name;
}

template< class T >
string testNs( T *t ) {
    stringstream ss;
    ss << testDb( t ) << ".perftest";
    return ss.str();
}

template <class T>
class Runner {
public:
    void run() {
        T test;
        string name = testDb( &test );
        boost::posix_time::ptime start = boost::posix_time::microsec_clock::universal_time();
        test.run();
        boost::posix_time::ptime end = boost::posix_time::microsec_clock::universal_time();
        cout << name << ": " << end - start << endl;
    }
    ~Runner() {
        client_->dropDatabase( testDb< T >().c_str() );        
    }
};

class RunnerSuite : public UnitTest::Suite {
protected:
    template< class T >
    void add() {
        UnitTest::Suite::add< Runner< T > >();
    }
};

namespace Insert {
    class NoIndex {
    public:
        void run() {
            string ns = testNs( this );
            for( int i = 0; i < 100000; ++i )
                client_->insert( ns.c_str(), BSON( "_id" << i ) );
        }
    };

    class OneIndex {
    public:
        OneIndex() : ns_( testNs( this ) ) {
            client_->ensureIndex( ns_, BSON( "_id" << 1 ) );
        }
        void run() {
            for( int i = 0; i < 100000; ++i )
                client_->insert( ns_.c_str(), BSON( "_id" << i ) );
        }
        string ns_;
    };

    class TenIndex {
    public:
        TenIndex() : ns_( testNs( this ) ) {
            const char *names = "aaaaaaaaaa";
            for( int i = 0; i < 10; ++i ) {
                client_->resetIndexCache();
                client_->ensureIndex( ns_.c_str(), BSON( "_id" << 1 ), names + i );
            }            
        }
        void run() {
            for( int i = 0; i < 100000; ++i )
                client_->insert( ns_.c_str(), BSON( "_id" << i ) );
        }
        string ns_;
    };    
    
    class Capped {
    public:
        Capped() : ns_( testNs( this ) ) {
            client_->createCollection( ns_.c_str(), 100000, true );
        }
        void run() {
            for( int i = 0; i < 100000; ++i )
                client_->insert( ns_.c_str(), BSON( "_id" << i ) );
        }
        string ns_;
    };

    class OneIndexReverse {
    public:
        OneIndexReverse() : ns_( testNs( this ) ) {
            client_->ensureIndex( ns_, BSON( "_id" << 1 ) );
        }
        void run() {
            for( int i = 0; i < 100000; ++i )
                client_->insert( ns_.c_str(), BSON( "_id" << ( 100000 - 1 - i ) ) );
        }
        string ns_;
    };    
    
    class OneIndexHighLow {
    public:  
        OneIndexHighLow() : ns_( testNs( this ) ) {
            client_->ensureIndex( ns_, BSON( "_id" << 1 ) );
        }
        void run() {
            for( int i = 0; i < 100000; ++i ) {
                int j = 50000 + ( ( i % 2 == 0 ) ? 1 : -1 ) * ( i / 2 + 1 );
                client_->insert( ns_.c_str(), BSON( "_id" << j ) );
            }
        }
        string ns_;
    };
    
    class All : public RunnerSuite {
    public:
        All() {
            add< NoIndex >();
            add< OneIndex >();
            add< TenIndex >();
            add< Capped >();
            add< OneIndexReverse >();
            add< OneIndexHighLow >();
        }
    };
} // namespace Insert

namespace Update {
    class Smaller {
    public:
        Smaller() : ns_( testNs( this ) ) {
            client_->ensureIndex( ns_, BSON( "_id" << 1 ) );
            for( int i = 0; i < 100000; ++i )
                client_->insert( ns_.c_str(), BSON( "_id" << i << "b" << 2 ) );            
        }
        void run() {
            for( int i = 0; i < 100000; ++i )
                client_->update( ns_.c_str(), QUERY( "_id" << i ), BSON( "_id" << i ) );
        }
        string ns_;
    };
    
    class Bigger {
    public:
        Bigger() : ns_( testNs( this ) ) {
            client_->ensureIndex( ns_, BSON( "_id" << 1 ) );
            for( int i = 0; i < 100000; ++i )
                client_->insert( ns_.c_str(), BSON( "_id" << i ) );            
        }
        void run() {
            for( int i = 0; i < 100000; ++i )
                client_->update( ns_.c_str(), QUERY( "_id" << i ), BSON( "_id" << i << "b" << 2 ) );
        }
        string ns_;
    };

    class All : public RunnerSuite {
    public:
        All() {
            add< Smaller >();
            add< Bigger >();
        }
    };
} // namespace Update

namespace BSON {
    
    const char *sample =
    "{\"one\":2, \"two\":5, \"three\": {},"
    "\"four\": { \"five\": { \"six\" : 11 } },"
    "\"seven\": [ \"a\", \"bb\", \"ccc\", 5 ],"
    "\"eight\": Dbref( \"rrr\", \"01234567890123456789aaaa\" ),"
    "\"_id\": ObjectId( \"deadbeefdeadbeefdeadbeef\" ),"
    "\"nine\": { \"$binary\": \"abc=\", \"$type\": \"02\" },"
    "\"ten\": Date( 44 ), \"eleven\": /foooooo/i }";
    
    const char *shopwikiSample =
    "{ '_id' : '289780-80f85380b5c1d4a0ad75d1217673a4a2' , 'site_id' : 289780 , 'title'"
    ": 'Jubilee - Margaret Walker' , 'image_url' : 'http://www.heartlanddigsandfinds.c"
    "om/store/graphics/Product_Graphics/Product_8679.jpg' , 'url' : 'http://www.heartla"
    "nddigsandfinds.com/store/store_product_detail.cfm?Product_ID=8679&Category_ID=2&Su"
    "b_Category_ID=910' , 'url_hash' : 3450626119933116345 , 'last_update' :  null  , '"
    "features' : { '$imagePrefetchDate' : '2008Aug30 22:39' , '$image.color.rgb' : '5a7"
    "574' , 'Price' : '$10.99' , 'Description' : 'Author--s 1st Novel. A Houghton Miffl"
    "in Literary Fellowship Award novel by the esteemed poet and novelist who has demon"
    "strated a lifelong commitment to the heritage of black culture. An acclaimed story"
    "of Vyry, a negro slave during the 19th Century, facing the biggest challenge of h"
    "er lifetime - that of gaining her freedom, fighting for all the things she had nev"
    "er known before. The author, great-granddaughter of Vyry, reveals what the Civil W"
    "ar in America meant to the Negroes. Slavery W' , '$priceHistory-1' : '2008Dec03 $1"
    "0.99' , 'Brand' : 'Walker' , '$brands_in_title' : 'Walker' , '--path' : '//HTML[1]"
    "/BODY[1]/TABLE[1]/TR[1]/TD[1]/P[1]/TABLE[1]/TR[1]/TD[1]/TABLE[1]/TR[2]/TD[2]/TABLE"
    "[1]/TR[1]/TD[1]/P[1]/TABLE[1]/TR[1]' , '~location' : 'en_US' , '$crawled' : '2009J"
    "an11 03:22' , '$priceHistory-2' : '2008Nov15 $10.99' , '$priceHistory-0' : '2008De"
    "c24 $10.99'}}";
    
    class Parse {
    public:
        void run() {
            for( int i = 0; i < 10000; ++i )
                fromjson( sample );
        }
    };
    
    class ShopwikiParse {
    public:
        void run() {
            for( int i = 0; i < 10000; ++i )
                fromjson( shopwikiSample );
        }
    };
    
    class Json {
    public:
        Json() : o_( fromjson( sample ) ) {}
        void run() {
            for( int i = 0; i < 10000; ++i )
                o_.jsonString();
        }
        BSONObj o_;
    };

    class ShopwikiJson {
    public:
        ShopwikiJson() : o_( fromjson( shopwikiSample ) ) {}
        void run() {
            for( int i = 0; i < 10000; ++i )
                o_.jsonString();
        }
        BSONObj o_;
    };

    class All : public RunnerSuite {
    public:
        All() {
            add< Parse >();
            add< ShopwikiParse >();
            add< Json >();
            add< ShopwikiJson >();
        }
    };
    
} // namespace BSON

namespace Index {

    class Int {
    public:
        Int() : ns_( testNs( this ) ) {
            for( int i = 0; i < 100000; ++i )
                client_->insert( ns_.c_str(), BSON( "_id" << i ) );
        }
        void run() {
            client_->ensureIndex( ns_, BSON( "_id" << 1 ) );
        }
        string ns_;
    };

    class ObjectId {
    public:
        ObjectId() : ns_( testNs( this ) ) {
            OID id;
            for( int i = 0; i < 100000; ++i ) {
                id.init();
                client_->insert( ns_.c_str(), BSON( "_id" << id ) );
            }
        }
        void run() {
            client_->ensureIndex( ns_, BSON( "_id" << 1 ) );
        }
        string ns_;
    };

    class String {
    public:
        String() : ns_( testNs( this ) ) {
            for( int i = 0; i < 100000; ++i ) {
                stringstream ss;
                ss << i;
                client_->insert( ns_.c_str(), BSON( "_id" << ss.str() ) );
            }
        }
        void run() {
            client_->ensureIndex( ns_, BSON( "_id" << 1 ) );
        }
        string ns_;
    };

    class Object {
    public:
        Object() : ns_( testNs( this ) ) {
            for( int i = 0; i < 100000; ++i ) {
                client_->insert( ns_.c_str(), BSON( "_id" << BSON( "a" << i ) ) );
            }
        }
        void run() {
            client_->ensureIndex( ns_, BSON( "_id" << 1 ) );
        }
        string ns_;
    };    
    
    class All : public RunnerSuite {
    public:
        All() {
            add< Int >();
            add< ObjectId >();
            add< String >();
            add< Object >();
        }
    };
    
} // namespace Index

namespace QueryTests {
    
    class NoMatch {
    public:
        NoMatch() : ns_( testNs( this ) ) {
            for( int i = 0; i < 100000; ++i )
                client_->insert( ns_.c_str(), BSON( "_id" << i ) );
        }
        void run() {
            client_->findOne( ns_.c_str(), QUERY( "_id" << 100000 ) );
        }
        string ns_;
    };
    
    class NoMatchIndex {
    public:
        NoMatchIndex() : ns_( testNs( this ) ) {
            client_->ensureIndex( ns_, BSON( "_id" << 1 ) );
            for( int i = 0; i < 100000; ++i )
                client_->insert( ns_.c_str(), BSON( "_id" << i ) );
        }
        void run() {
            client_->findOne( ns_.c_str(),
                             QUERY( "a" << "b" ).hint( BSON( "_id" << 1 ) ) );
        }
        string ns_;
    };

    class NoMatchLong {
    public:
        NoMatchLong() : ns_( testNs( this ) ) {
            const char *names = "aaaaaaaaaa";
            for( int i = 0; i < 100000; ++i ) {
                BSONObjBuilder b;
                for( int j = 0; j < 10; ++j )
                    b << ( names + j ) << i;
                client_->insert( ns_.c_str(), b.obj() );
            }
        }
        void run() {
            client_->findOne( ns_.c_str(), QUERY( "a" << 100000 ) );
        }
        string ns_;
    };    
    
    class SortOrdered {
    public:
        SortOrdered() : ns_( testNs( this ) ) {
            for( int i = 0; i < 50000; ++i )
                client_->insert( ns_.c_str(), BSON( "_id" << i ) );
        }
        void run() {
            auto_ptr< DBClientCursor > c = 
            client_->query( ns_.c_str(), Query( emptyObj ).sort( BSON( "_id" << 1 ) ) );
            int i = 0;
            for( ; c->more(); c->nextSafe(), ++i );
            ASSERT_EQUALS( 50000, i );
        }
        string ns_;
    };
    
    class SortReverse {
    public:
        SortReverse() : ns_( testNs( this ) ) {
            for( int i = 0; i < 50000; ++i )
                client_->insert( ns_.c_str(), BSON( "_id" << ( 50000 - 1 - i ) ) );
        }
        void run() {
            auto_ptr< DBClientCursor > c = 
            client_->query( ns_.c_str(), Query( emptyObj ).sort( BSON( "_id" << 1 ) ) );
            int i = 0;
            for( ; c->more(); c->nextSafe(), ++i );
            ASSERT_EQUALS( 50000, i );
        }
        string ns_;
    };

    class All : public RunnerSuite {
    public:
        All() {
            add< NoMatch >();
            add< NoMatchIndex >();
            add< NoMatchLong >();
            add< SortOrdered >();
            add< SortReverse >();
        }
    };    
    
} // namespace QueryTests

namespace Plan {

    class Hint {
    public:
        Hint() : ns_( testNs( this ) ) {
            const char *names = "aaaaaaaaaa";
            for( int i = 0; i < 10; ++i ) {
                client_->resetIndexCache();
                client_->ensureIndex( ns_.c_str(), BSON( ( names + i ) << 1 ), names + i );
            }
            lk_.reset( new dblock );
            setClient( ns_.c_str() );
            hint_ = BSON( "hint" << BSON( "a" << 1 ) );
            hintElt_ = hint_.firstElement();            
        }
        void run() {
            for( int i = 0; i < 10000; ++i )
                getIndexCursor( ns_.c_str(), emptyObj, emptyObj, 0, 0, &hintElt_ );
        }
        string ns_;        
        auto_ptr< dblock > lk_;
        BSONObj hint_;
        BSONElement hintElt_;
    };
    
    class Sort {
    public:
        Sort() : ns_( testNs( this ) ) {
            const char *names = "aaaaaaaaaa";
            for( int i = 0; i < 10; ++i ) {
                client_->resetIndexCache();
                client_->ensureIndex( ns_.c_str(), BSON( ( names + i ) << 1 ), names + i );
            }
            lk_.reset( new dblock );
            setClient( ns_.c_str() );
        }
        void run() {
            for( int i = 0; i < 10000; ++i )
                getIndexCursor( ns_.c_str(), emptyObj, BSON( "a" << 1 ) );
        }
        string ns_;        
        auto_ptr< dblock > lk_;
    };

    class Query {
    public:
        Query() : ns_( testNs( this ) ) {
            const char *names = "aaaaaaaaaa";
            for( int i = 0; i < 10; ++i ) {
                client_->resetIndexCache();
                client_->ensureIndex( ns_.c_str(), BSON( ( names + i ) << 1 ), names + i );
            }
            lk_.reset( new dblock );
            setClient( ns_.c_str() );
        }
        void run() {
            for( int i = 0; i < 10000; ++i )
                getIndexCursor( ns_.c_str(), BSON( "a" << 1 ), emptyObj );
        }
        string ns_;        
        auto_ptr< dblock > lk_;
    };
    
    class All : public RunnerSuite {
    public:
        All() {
            add< Hint >();
            add< Sort >();
            add< Query >();
        }
    };    
    
} // namespace Plan

template< class T >
UnitTest::TestPtr suite() {
    return UnitTest::createSuite< T >();
}

int main( int argc, char **argv ) {
    
    logLevel = -1;
    
    boost::filesystem::path p( "/data/db/perftest" );
    if ( boost::filesystem::exists( p ) )
        boost::filesystem::remove_all( p );
    boost::filesystem::create_directory( p );
    dbpath = p.native_directory_string().c_str();
    
    client_ = new DBDirectClient();
    
    UnitTest::Registry tests;
    tests.add( suite< Insert::All >(), "insert" );
    tests.add( suite< Update::All >(), "update" );
    tests.add( suite< BSON::All >(), "bson" );
    tests.add( suite< Index::All >(), "index" );
    tests.add( suite< QueryTests::All >(), "query" );
    tests.add( suite< Plan::All >(), "plan" );

    return tests.run( argc, argv );    
}