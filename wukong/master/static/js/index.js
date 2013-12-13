var app = angular.module('applications', ['ngResource', 'ui.router']);

app.config(function($stateProvider, $urlRouterProvider) {
  //$urlRouterProvider.otherwise('/');

  $stateProvider
    .state('index', {
      url: '',
      views: {
        'list': {
          templateUrl: 'static/partials/applications_list.html'
        }
      }
    })
    .state('select', {
      url: '/a/:appId',
      views: {
        'list': {
          templateUrl: 'static/partials/applications_list.html'
        },
        'body': {
          templateUrl: function(stateParams) {
            return '/applications/' + stateParams.appId;
          }
        }
      }
    })
});

app.factory('Applications', function($resource) {
  return $resource('/applications/:appId', {appId: '@id'});
});

app.controller('MainController', function($scope, $state, $log, Applications) {
  $scope.apps = Applications.query();
  $scope.create = function() {
    app_name = prompt('Please enter the application name:', 'Application name');
    var newApp = new Applications({app_name: app_name});
    $log.debug('creating');
    newApp.$save({app_name: app_name}, function() {
      $log.debug('saved');
      $log.debug(newApp);
      $scope.apps = Applications.query();
      window.location.href = '/#/a/' + newApp.id; // angularjs routing prefix
    });
  };
  $scope.remove = function(app) {
    app.$remove();
    // It should update $scope.apps automatically
    window.location.href = '/';
  };
});
