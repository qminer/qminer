function StoreCtrl($scope, $http) {
    $scope.stores = [ ];
    
    // get list of stores from qminer
    $http.get('../qm_stores').success(function (data) {
        $scope.stores = data;
    });
}
