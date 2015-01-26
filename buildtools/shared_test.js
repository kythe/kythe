'use strict';

var assert = require('assert');
var fs = require('fs');
var path = require('path');

var tmp = require('tmp');
var data_driven = require('data-driven');

var shared = require('./shared.js');

describe('Array', function() {
  describe('#append()', function() {
    it('Basic append functionality', function() {
      assert.deepEqual([1, 2], [1].append([2]));
      assert.deepEqual([1], [1].append([]));
      assert.deepEqual([1], [].append([1]));
      assert.deepEqual([], [].append([]));
      assert.deepEqual([1], [1].append(undefined));
      assert.deepEqual([1], [1].append(null));
      assert.deepEqual([1, undefined], [1].append([undefined]));
      assert.throws(function() { [1].append('a'); }, Error);

      var x = [1, 2];
      var y = [3];
      var z = x.append(y);
      assert.deepEqual([1, 2, 3], z);
      assert.deepEqual([3], y);
      assert.equal(x, z);
    });
  });
});

describe('String', function() {
  describe('#startsWith()', function() {
    it('Basic startsWith functionality', function() {
      assert.equal(true, 'abcd'.startsWith('ab'));
      assert.equal(true, 'ab'.startsWith('ab'));
      assert.equal(false, 'ab'.startsWith('abc'));
      assert.equal(false, 'ab'.startsWith('cd'));
      assert.equal(false, ''.startsWith('ab'));
      // odd man out
      assert.equal(true, 'ab'.startsWith(''));
      // automagic conversions
      assert.equal(true, '1'.startsWith(1));
      assert.equal(true, 'true'.startsWith(true));
    });
  });
  describe('#endsWith()', function() {
    it('Basic endsWith functionality', function() {
      assert.equal(true, 'abcd'.endsWith('cd'));
      assert.equal(true, 'ab'.endsWith('ab'));
      assert.equal(false, 'ab'.endsWith('abc'));
      assert.equal(false, 'ab'.endsWith('cd'));
      assert.equal(false, ''.endsWith('ab'));
      // odd man out
      assert.equal(true, 'ab'.endsWith(''));
      // automagic conversions
      assert.equal(true, '1'.endsWith(1));
      assert.equal(true, 'true'.endsWith(true));
    });
  });
});

function createFileLayout(base, layout) {
  for (var i = 0; i < layout.length; i++) {
    var entry = layout[i];
    var fn = path.join(base, entry.name);
    switch (entry.type) {
      case 'file':
      fs.writeFileSync(fn, entry.content, entry.options);
      break;
      case 'directory':
      fs.mkdirSync(fn, entry.mode);
      if (entry.content) {
        createFileLayout(fn, entry.content);
      }
      break;
      default:
      throw new Error('unknown type for layout: ' + entry.type);
    }
  }
}

describe('#rmdirs()', function() {
  var tests = [
    {
      name: 'simple',
      layout: [{name: 'f', type: 'file', content: 'content'}]
    },
    {
      name: 'nested',
      layout: [
        {
          name: 'd',
          type: 'directory',
          content: [{name: 'f', type: 'file', content: 'text'}]
        }
      ]
    },
    {
      name: 'deeply nested',
      layout: [
        {
          name: 'd', type: 'directory', content: [
            {name: 'f', type: 'file', content: 'data'},
            {name: 'g', type: 'file', content: 'bla'},
            {
              name: 'd2',
              type: 'directory',
              content: [{name: 'f', type: 'file', content: 'baz'}]
            }
          ]
        }
      ]
    },
    {
      name: 'empty dir',
      layout: [
        {name: 'd', type: 'directory', content: []},
        {name: 'f', type: 'file', content: ''}
      ]
    },
    {
      name: 'extensions',
      layout: [{
        name: 'd',
        type: 'directory',
        content: [
          {name: 'file1.txt', type: 'file', content: ''},
          {name: 'file2.txt', type: 'file', content: ''},
          {name: 'file3.csv', type: 'file', content: ''},
          {name: 'file4.xml', type: 'file', content: ''},
          {name: 'file6.txt', type: 'file', content: ''}
        ]
      }]
    }
  ];

  data_driven(tests, function() {
    it('rmdirs {name}', function(ctx, done) {
      // tmpdir will fail if files are left behind.
      tmp.dir({unsafeCleanup: false},
              function _tempDirCreated(err, base, cleanupCallback) {
                if (err) throw err;

                var root = path.join(base, 'root');
                fs.mkdirSync(root);
                createFileLayout(root, ctx.layout);
                var results = shared.rmdirs(root);
                cleanupCallback();
                done();
              });
    });
  });
});

describe('#parseVersion()', function() {
  it('Basic parseVersion functionality', function() {
    var version = shared.parseVersion('1.2.3');
    assert.equal(1, version.major);
    assert.equal(2, version.minor);
    assert.equal(3, version.patch);
    assert.throws(function() { shared.parseVersion('1.2'); }, Error);
    assert.throws(function() { shared.parseVersion('1.2.3.4'); }, Error);
  });
});
