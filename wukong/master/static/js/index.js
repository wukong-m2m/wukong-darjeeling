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
    .state('select_example', {
      url: '/a/select/:appId', 
      views: { 
        'list' : {
          templateUrl: 'static/partials/applications_list.html'
        },
        'body' : {
          templateUrl: function(stateParams) {
            alert(""+str(stateParams.appId));
            return '/applications/' + stateParams.appId;
          }
        }
      }
    })
});

app.factory('Applications', function($resource) {
  return $resource('/applications/:appId', {appId: '@id'});
});

//this service is for querying example applications
app.factory('ApplicationsEx', function($resource){
  return $resource('/example_applications/:appId:appName', {appId: '@id', appName: '@id'});
});

app.controller('MainController', function($scope, $state, $log, Applications, ApplicationsEx) {
  $scope.apps = Applications.query();
  $scope.ex_apps = ApplicationsEx.query();
  $scope.create = function() {
    app_name = prompt('Please enter the application name:', 'Application name');
    if(app_name != '' && app_name != null) {
      var newApp = new Applications({app_name: app_name});
      $log.debug('creating');
      newApp.$save({app_name: app_name}, function() {
        $log.debug('saved');
        $log.debug(newApp);
        $scope.apps = Applications.query();
        window.location.href = '/#/a/' + newApp.id; // angularjs routing prefix
      });
    }
  };
  $scope.select_example = function(app) {
    var default_name = prompt('Please provide new name for the application?\n(This will copy example application to your local folder, you can just cancel it.)', "");
    if (default_name != null && default_name != ""){
      var newApp = new ApplicationsEx();
      newApp.$save({app_name: default_name, app_id:app.id}, function() {
        if(newApp.id != null){ // successfully create application
          $scope.apps = Applications.query();
          window.location.href = '/#/a/' + newApp.id; // heading to the new application
        }
        else{
          if (newApp['status'] == '1'){
            alert(newApp['mesg']);
          }
        }
      });
    }
  };
  $scope.remove = function(app) {
    app.$remove();
    // It should update $scope.apps automatically
    window.location.href = '/';
  };
});
