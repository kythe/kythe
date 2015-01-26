'use strict';

var entity = require('./entity.js');
var rule = require('./rule.js');
var shared = require('./shared.js');

// The DockerDeploy rule builds/pushes a Docker image with a context directory
// containing the outputs of the 'inputs.data' targets/files.  Each target
// requires the 'docker_name' property for packaging and the 'docker_namespace'
// property for deployment. The 'docker_tag' property is optional for both.
function DockerDeploy(engine) {
  this.engine = engine;
}
DockerDeploy.prototype = new rule.Rule();
DockerDeploy.prototype.getNinjaBuilds = function(target) {
  var srcs = rule.getAllOutputsFor(target.inputsByKind['srcs'], 'build',
                                   rule.fileFilter('src_file', '/Dockerfile'));
  var data = rule.getAllOutputsFor(target.inputsByKind['data'], 'build',
                                   rule.allFilesAndDirsFilter);
  var deps = rule.getAllOutputsFor(target.inputsByKind['deps'], 'package',
                                   rule.allFilesFilter);
  var localName = getLocalName(target) + getTag(target);
  var buildDone =
      target.getFileNode(target.getRoot('gen') + '.done', 'done_marker');

  var tags = {'latest': true};
  var tag = target.getProperty('docker_tag');
  if (tag && tag.value) {
    tags[tag.value] = true;
  }
  var releaseVersion = target.getProperty('release_version');
  if (releaseVersion && releaseVersion.value) {
    var semVer = shared.parseVersion(releaseVersion.value);
    tags[semVer.major] = true;
    tags[semVer.major + '.' + semVer.minor] = true;
    tags[semVer.major + '.' + semVer.minor + '.' + semVer.patch] = true;
  }
  var remoteName = getRemoteName(target);

  return {
    PACKAGE: [{
      rule: 'docker_build',
      inputs: srcs.concat(data),
      implicits: deps,
      outs: [buildDone],
      vars: {
        outdir: target.getRoot('gen'),
        name: localName
      }
    }],
    DEPLOY: Object.keys(tags).map(function(tag) {
      return {
        rule: 'docker_push',
        inputs: [],
        implicits: [buildDone],
        outs: [target.getFileNode(target.getRoot('gen') + '/push_' + tag + '_done',
                               'done_marker')],
        vars: {
          local: localName,
          remote: remoteName + ':' + tag
        }
      };
    })
  };
};

// The DockerImage rule pulls a Docker image from a particular repository. Each
// target requires the 'docker_namespace' and 'docker_name' properties. The
// 'docker_tag' property is optional.
function DockerImage(engine) {
  this.engine = engine;
}
DockerImage.prototype = new rule.Rule();
DockerImage.prototype.getNinjaBuilds = function(target) {
  var localName = getLocalName(target) + getTag(target);
  var remoteName = getRemoteName(target) + getTag(target);
  return {
    PACKAGE: [{
      rule: 'docker_pull',
      inputs: [],
      outs: [target.getFileNode(target.getRoot('gen') + '.done', 'done_marker')],
      vars: {
        local: localName,
        remote: remoteName
      }
    }]
  };
};

function getLocalName(target) {
  return target.getProperty('docker_name').value;
}

function getRemoteName(target) {
  var name = target.getProperty('docker_name');
  var namespace = target.getProperty('docker_namespace');
  if (!name || !name.value) {
    console.error('ERROR: ' + target.id +
        ' is missing the docker_name property');
    process.exit(1);
  } else if (!namespace || !namespace.value) {
    console.error('ERROR: ' + target.id +
        ' is missing the docker_namespace property');
    process.exit(1);
  }

  var tokens = name.value.split('/');
  return namespace.value + '/' + tokens[tokens.length - 1];
}

function getTag(target) {
  var tag = target.getProperty('docker_tag');
  if (tag && tag.value) {
    return ':' + tag.value;
  } else {
    return ':latest';
  }
}

exports.register = function(engine) {
  engine.addRule('docker_deploy', new DockerDeploy(engine));
  engine.addRule('docker_image', new DockerImage(engine));
};
