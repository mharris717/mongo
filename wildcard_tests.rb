require 'rubygems'
require 'mongo'
require 'spec'
require 'mongo_scope'

def db
  $db ||= Mongo::Connection.new.db('wildcard-test')
end

context 'wildcard' do
  context 'equality' do
    before do
      coll = db.collection('abc')
      coll.remove
      coll.save(:a => {:b => {:c => 1}})
      coll.save(:a => {:b => {:c => 2}})
      coll.save(:a => {:b => {:c => 3}})
      @coll = coll
    end
    it 'matches no wc' do
      @coll.find('a.b.c' => 2).count.should == 1
    end
    it 'matches wc at begin' do
      @coll.find('*.b.c' => 2).count.should == 1
    end
    it 'matches wc in middle' do
      @coll.find('a.*.c' => 2).count.should == 1
    end
    it 'matches wc at end' do
      @coll.find('a.b.*' => 2).count.should == 1
    end
    it 'matches wc multiple' do
      @coll.find('a.*.*' => 2).count.should == 1
    end
    it 'matches all wc' do
      @coll.find('*.*.*' => 2).count.should == 1
    end
  end
  context 'equality array at end' do
    before do
      @coll = db.collection('abc')
      @coll.remove
      @coll.save(:a => {:b => {:c => [1,2]}})
      @coll.save(:a => {:b => {:c => [3,4]}})
      @coll.save(:a => {:b => {:c => [5,6]}})
    end
    it 'matches no wc' do
      @coll.find('a.b.c' => 2).count.should == 1
    end
    it 'matches wc at begin' do
      @coll.find('*.b.c' => 2).count.should == 1
    end
    it 'matches wc in middle' do
      @coll.find('a.*.c' => 2).count.should == 1
    end
    it 'matches wc at end' do
      @coll.find('a.b.*' => 2).count.should == 1
    end
    it 'matches wc multiple' do
      @coll.find('a.*.*' => 2).count.should == 1
    end
    it 'matches all wc' do
      @coll.find('*.*.*' => 2).count.should == 1
    end
  end
  context 'all operator' do
    # none of the wildcard tests pass
    before do
      @coll = db.collection('abc')
      @coll.remove
      @coll.save(:a => {:b => {:c => [1,2,3]}})
      @coll.save(:a => {:b => {:c => [2,3,4]}})
      @coll.save(:a => {:b => {:c => [3,4,5]}})
    end
    it 'matches no wc' do
      @coll.scope_all('a.b.c' => [3,4]).count.should == 2
    end
    it 'matches wc at begin' do
      pending "$all doesn't work with wildcards"
      @coll.scope_all('*.b.c' => [3,4]).count.should == 2
    end
    it 'matches wc in middle' do
      pending "$all doesn't work with wildcards"
      @coll.scope_all('a.*.c' => [3,4]).count.should == 2
    end
    it 'matches wc at end' do
      pending "$all doesn't work with wildcards"
      @coll.scope_all('a.b.*' => [3,4]).count.should == 2
    end
    it 'matches wc multiple' do
      pending "$all doesn't work with wildcards"
      @coll.scope_all('a.*.*' => [3,4]).count.should == 2
    end
    it 'matches all wc' do
      pending "$all doesn't work with wildcards"
      @coll.scope_all('*.*.*' => [3,4]).count.should == 2
    end
  end
  context 'equality array in middle' do
    before do
      @coll = db.collection('abc')
      @coll.remove
      @coll.save(:a => [{:b => {:c => 1}},{:b => {:c => 2}}])
      @coll.save(:a => [{:b => {:c => 3}},{:b => {:c => 4}}])
      @coll.save(:a => [{:b => {:c => 5}},{:b => {:c => 6}}])
    end
    it 'matches no wc' do
      @coll.find('a.b.c' => 2).count.should == 1
    end
    it 'matches wc at begin' do
      @coll.find('*.b.c' => 2).count.should == 1
    end
    it 'matches wc in middle' do
      @coll.find('a.*.c' => 2).count.should == 1
    end
    it 'matches wc at end' do
      @coll.find('a.b.*' => 2).count.should == 1
    end
    it 'matches wc multiple' do
      @coll.find('a.*.*' => 2).count.should == 1
    end
    it 'matches all wc' do
      @coll.find('*.*.*' => 2).count.should == 1
    end
  end
  context 'multiple arrays' do
    before do
      @coll = db.collection('abc')
      @coll.remove
      @coll.save(:a => [{:b => [{:c => 1},{:c => 2}]},{:b => [{:c => 3},{:c => 4}]}])
      @coll.save(:a => [{:b => [{:c => 5},{:c => 6}]},{:b => [{:c => 7},{:c => 8}]}])
      @coll.save(:a => [{:b => [{:c => 9},{:c => 10}]},{:b => [{:c => 11},{:c => 12}]}])
    end
    it 'matches no wc' do
      @coll.find('a.b.c' => 2).count.should == 1
    end
    it 'matches wc at begin' do
      @coll.find('*.b.c' => 2).count.should == 1
    end
    it 'matches wc in middle' do
      @coll.find('a.*.c' => 2).count.should == 1
    end
    it 'matches wc at end' do
      @coll.find('a.b.*' => 2).count.should == 1
    end
    it 'matches wc multiple' do
      @coll.find('a.*.*' => 2).count.should == 1
    end
    it 'matches all wc' do
      @coll.find('*.*.*' => 2).count.should == 1
    end
  end
  context 'multiple arrays with missing values' do
    before do
      @coll = db.collection('abc')
      @coll.remove
      @coll.save(:a => [{:b => [{:c => 1},{:d => 2}]},{:b => [{:c => 3},{:d => 4}]}])
      @coll.save(:a => [{:b => [{:c => 5},{:d => 6}]},{:b => [{:c => 7},{:d => 8}]}])
      @coll.save(:a => [{:b => [{:c => 9},{:d => 10}]},{:b => [{:c => 11},{:d => 12}]}])
    end
    it 'matches no wc' do
      @coll.find('a.b.c' => 3).count.should == 1
    end
    it 'matches wc at begin' do
      @coll.find('*.b.c' => 3).count.should == 1
    end
    it 'matches wc in middle' do
      @coll.find('a.*.c' => 3).count.should == 1
    end
    it 'matches wc at end' do
      @coll.find('a.b.*' => 3).count.should == 1
    end
    it 'matches wc multiple' do
      @coll.find('a.*.*' => 3).count.should == 1
    end
    it 'matches all wc' do
      @coll.find('*.*.*' => 3).count.should == 1
    end
  end
  context 'greater than' do
    before do
      @coll = db.collection('abc')
      @coll.remove
      @coll.save(:a => {:b => {:c => 2}})
      @coll.save(:a => {:b => {:c => 4}})
      @coll.save(:a => {:b => {:c => 6}})
    end
    it 'matches no wc' do
      @coll.scope_gt('a.b.c' => 5).count.should == 1
    end
    it 'matches wc at begin' do
      @coll.scope_gt('*.b.c' => 5).count.should == 1
    end
    it 'matches wc in middle' do
      @coll.scope_gt('a.*.c' => 5).count.should == 1
    end
    it 'matches wc at end' do
      @coll.scope_gt('a.b.*' => 5).count.should == 1
    end
    it 'matches wc multiple' do
      @coll.scope_gt('a.*.*' => 5).count.should == 1
    end
    it 'matches all wc' do
      @coll.scope_gt('*.*.*' => 5).count.should == 1
    end
  end
  context 'not equal' do
    before do
      @coll = db.collection('abc')
      @coll.remove
      @coll.save(:a => {:b => {:c => 2}})
      @coll.save(:a => {:b => {:c => 4}})
      @coll.save(:a => {:b => {:c => 6}})
    end
    it 'matches no wc' do
      @coll.scope_ne('a.b.c' => 2).count.should == 2
    end
    it 'matches wc at begin' do
      @coll.scope_ne('*.b.c' => 2).count.should == 2
    end
    it 'matches wc in middle' do
      @coll.scope_ne('a.*.c' => 2).count.should == 2
    end
    it 'matches wc at end' do
      @coll.scope_ne('a.b.*' => 2).count.should == 2
    end
    it 'matches wc multiple' do
      @coll.scope_ne('a.*.*' => 2).count.should == 2
    end
    it 'matches all wc' do
      @coll.scope_ne('*.*.*' => 2).count.should == 2
    end
  end
  context 'in basic' do
    before do
      @coll = db.collection('abc')
      @coll.remove
      @coll.save(:a => {:b => {:c => [1,2]}})
      @coll.save(:a => {:b => {:c => [3,4]}})
      @coll.save(:a => {:b => {:c => [5,6]}})
    end
    it 'matches no wc' do
      @coll.scope_in('a.b.c' => [2,3]).count.should == 2
    end
    it 'matches wc at begin' do
      @coll.scope_in('*.b.c' => [2,3]).count.should == 2
    end
    it 'matches wc in middle' do
      @coll.scope_in('a.*.c' => [2,3]).count.should == 2
    end
    it 'matches wc at end' do
      @coll.scope_in('a.b.*' => [2,3]).count.should == 2
    end
    it 'matches wc multiple' do
      @coll.scope_in('a.*.*' => [2,3]).count.should == 2
    end
    it 'matches all wc' do
      @coll.scope_in('*.*.*' => [2,3]).count.should == 2
    end
  end
  context 'nin basic' do
    before do
      @coll = db.collection('abc')
      @coll.remove
      @coll.save(:a => {:b => {:c => [1,2]}})
      @coll.save(:a => {:b => {:c => [3,4]}})
      @coll.save(:a => {:b => {:c => [5,6]}})
    end
    it 'matches no wc' do
      @coll.scope_nin('a.b.c' => [2,3]).count.should == 1
    end
    it 'matches wc at begin' do
      @coll.scope_nin('*.b.c' => [2,3]).count.should == 1
    end
    it 'matches wc in middle' do
      @coll.scope_nin('a.*.c' => [2,3]).count.should == 1
    end
    it 'matches wc at end' do
      @coll.scope_nin('a.b.*' => [2,3]).count.should == 1
    end
    it 'matches wc multiple' do
      @coll.scope_nin('a.*.*' => [2,3]).count.should == 1
    end
    it 'matches all wc' do
      @coll.scope_nin('*.*.*' => [2,3]).count.should == 1
    end
  end
  context 'regex' do
    before do
      @coll = db.collection('abc')
      @coll.remove
      @coll.save(:a => {:b => {:c => 'a'}})
      @coll.save(:a => {:b => {:c => 'b'}})
      @coll.save(:a => {:b => {:c => 'c'}})
    end
    it 'normal no wc' do
      @coll.find('a.b.c' => /a/).count.should == 1
    end
    it 'matches wc at begin' do
      @coll.find('*.b.c' => /a/).count.should == 1
    end
    it 'matches wc in middle' do
      @coll.find('a.*.c' => /a/).count.should == 1
    end
    it 'matches wc at end' do
      @coll.find('a.b.*' => /a/).count.should == 1
    end
    it 'matches wc multiple' do
      @coll.find('a.*.*' => /a/).count.should == 1
    end
    it 'matches all wc' do
      @coll.find('*.*.*' => /a/).count.should == 1
    end
  end
end
