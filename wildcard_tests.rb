require 'rubygems'
require 'mongo'
require 'spec'
require 'mongo_scope'

def db
  Mongo::Connection.new.db('abc')
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
  end
  context 'equality array at end' do
    before do
      @coll = db.collection('abc')
      @coll.remove
      @coll.save(:a => {:b => {:c => [1,2]}})
      @coll.save(:a => {:b => {:c => [3,4]}})
      @coll.save(:a => {:b => {:c => [5,6]}})
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
  end
  context 'greater than' do
    before do
      @coll = db.collection('abc')
      @coll.remove
      @coll.save(:a => {:b => {:c => 2}})
      @coll.save(:a => {:b => {:c => 4}})
      @coll.save(:a => {:b => {:c => 6}})
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
  end
  context 'not equal' do
    before do
      @coll = db.collection('abc')
      @coll.remove
      @coll.save(:a => {:b => {:c => 2}})
      @coll.save(:a => {:b => {:c => 4}})
      @coll.save(:a => {:b => {:c => 6}})
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
  end
  context 'regex' do
    before do
      @coll = db.collection('abc')
      @coll.remove
      @coll.save(:a => {:b => {:c => 'a'}})
      @coll.save(:a => {:b => {:c => 'b'}})
      @coll.save(:a => {:b => {:c => 'c'}})
    end
    it 'normal wc at begin' do
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
  end
end
