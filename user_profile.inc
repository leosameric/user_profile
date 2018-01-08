<?php

//hook_form
function researcher_profile_form($form, &$form_state){
    
    $form = array();
    $form['name'] = array(
        '#title' => t("Researcher's name"), 
        '#type' =>'textfield', 
        '#weight' => 10,
        '#required' => TRUE,
        '#element_validate' => array('_user_profile_string_validate'),
    );
    $form['title'] = array(
        '#type' =>'textfield', 
        '#title' => t("Job title"), 
        '#weight' => 15,
    );
    $form['email'] = array(
        '#type' =>'textfield', 
        '#title' => t("Email address"), 
        '#weight' => 20,
        '#element_validate' => array('_user_profile_email_validate'),
    );
    $form['biography'] = array(
        '#type' =>'textarea', 
        '#title' => t("Biography"), 
        '#weight' => 25,
        '#description' => t('The maximum number of words for biography is 100.'),
        '#element_validate' => array('_user_profile_wordcount'),
    );
    $form['user_profile_image'] = array(
        '#type' => 'managed_file',
        '#name' => 'user_profile_image',
        '#title' => t('Block image'),
        '#size' => 40,
        '#upload_location' => 'public://',
        '#weight' => 98,
        '#description' => t("Image should be less than 1000*1000 pixels and in JPG, PNG, JPEG or GIF type."),
        '#upload_validators' => array(
            'file_validate_extensions' => array("png gif jpg jpeg"),
            'file_validate_size' => array(1*1000*1000),
        ),
    );
    
    $form['actions'] = array(
        '#type' => 'actions',
        '#weight' => 99,
    );
    $form['actions']['submit'] = array(
      '#type' => 'submit',
      '#value' => t('Submit'),
      '#submit' => array('researcher_profile_form_submit')
    );
    $form['actions']['cancel'] = array(
      '#markup' => l(t('Cancel'), ''),
    );
    
    return $form;
}

//hook_form_submit
function researcher_profile_form_submit($form, &$form_state) {
  
  global $user;
  $uid = $user->uid;
  
  //if user upload the image then save the image information into database.
  if (isset($form_state['values']['user_profile_image']) && $form_state['values']['user_profile_image'] != 0 ) {
    $file = file_load($form_state['values']['user_profile_image']);
    $file->status = FILE_STATUS_PERMANENT;
    file_save($file);
  }
  
  $nid = db_insert('user_profile') -> fields(array(
    'name' => $form_state['values']['name'],
    'title' => $form_state['values']['title'],
    'email' => $form_state['values']['email'],
    'biography' => $form_state['values']['biography'],
    'uid' => $uid,
    'created_time' => time(),
    'image_fid' => $form_state['values']['user_profile_image']
  ))->execute();
  
  drupal_goto($path = 'user_profile', $options = array());
}

function researcher_profile_list(){
    // retrieve user profile from database and display on the page.
    global $user;
    $uid = $user->uid;
    if($uid == 0){
        drupal_set_message(t('You are not a memeber. Please login or create a new account.'), 'status');
        drupal_goto($path = "", $options = array());
    }
    
    $result = db_query('
        SELECT u.name, u.title, u.email, u.biography, u.image_fid
        FROM {user_profile} u 
        WHERE u.uid = :uid 
        ORDER BY uid DESC LIMIT 1', array(':uid' => $uid)
    );
    foreach ($result as $record) {
      $name =      $record->name;
      $title =     $record->title;
      $email =     $record->email;
      $biography = $record->biography;
      $img_fid =   $record->image_fid;
    }
    if(!isset($name) || $name == ""){
        drupal_goto($path = 'user_profile/inputform', $options = array());
    }
    
    //if user uploaded image before, get image path.
    if($img_fid != 0){
      $img = file_load($img_fid)->uri;
    }
    
    drupal_add_css(drupal_get_path('module', 'user_profile') . '/user_profile.css', array('group' => CSS_DEFAULT));
    
    return 
      '<div class="r-frame">'
        .  '<div class="left-side">'
          . '<img id="r-img" src="'.(isset($img) ? file_create_url($img): file_create_url(drupal_get_path('module', 'user_profile').'/default-avatar.png')).'" alt="Img Figure">'
        . '</div>'
        . '<div class="right-side">'
          . '<div class="r-name">' .(isset($name) ? $name : "Anonymous"). '</div>'
          . '<div class="title-row">'
            . '<div class="r-title">' .(isset($title) ? $title : "Unknown"). '</div> '
            . '<div class="mid-line" style="border-left:1px solid #000;height:15px"> </div>'
            . '<div class="r-email"> <a href="mailto:'.(isset($email)?$email:"").'">'. (isset($email)?$email:"Unknown") .'</a> </div> '
          . '</div>' 
          . '<div class="frame-biography">'
            . '<div class="r-bio-title"><strong>Biography</strong></div>'
            . '<div class="r-biography">' .(isset($biography)?$biography:""). '</div>'
          . '</div>'
        . '</div>'
      . '</div>';
}

function _user_profile_wordcount($element, &$form_state){
    if (!empty($element['#value'])) {
        $number = str_word_count($element['#value']);
        if($number > 100){
          form_error($element, t('Total number of words in biography is "!num". The maxmum number of words is 100.', array('!num'=> $number)));
        }
    }
}

function _user_profile_email_validate($element, &$form_state){
    $email = $element['#value'];
    if (!valid_email_address($email)) {
      form_error($element, t('Email address format is wrong. Please enter a valid email address.'));
    }
}

function _user_profile_string_validate($element, &$form_state){
    $name = $element['#value'];
    if(!preg_match('/^[a-zA-Z\s]+$/', $name)){
        form_error($element, t('Your name contains numbers or special characters. Please retype your name.'));
    }
}