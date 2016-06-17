%==============================================================
% Image Processing Pipeline
% 
% This is a Matlab implementation of a pre-learned image 
% processing model. A description of the model can be found in
% "A New In-Camera Imaging Model for Color Computer Vision 
% and its Application" by Seon Joo Kim, Hai Ting Lin, 
% Michael Brown, et al. Code for learning a new model can 
% be found at the original project page.
%
% Original Project Page: 
% http://www.comp.nus.edu.sg/~brown/radiometric_calibration/
%
% Model Format Readme:
% http://www.comp.nus.edu.sg/~brown/radiometric_calibration/datasets/Model_param/readme.pdf
% 
%==============================================================

% Model directory
model_dir     = '../camera_models/NikonD40/Fl(L14)/';

% Image directory
image_dir     = '../imgs/';

% Input image
in_image_name = 'flower.NEF.raw_1C.tiff';

%==============================================================
% Import Forward Model Data
%
% Note: This assumes a camera model folder with a single 
% camera setting and transform. This is not the case for 
% every folder, but it is for the Nikon D40 on the Normal
% setting and with Fl(L14)/florescent color.

% Model file reading
transforms_file  = table2array( readtable( ...
    strcat(model_dir,'raw2jpg_transform.txt'), ...
    'Delimiter',' ','ReadVariableNames',false));
ctrl_points_file = table2array( readtable( ...
    strcat(model_dir,'raw2jpg_ctrlPoints.txt'), ...
    'Delimiter',' ','ReadVariableNames',false));
coeficients_file = table2array( readtable( ...
    strcat(model_dir,'raw2jpg_coefs.txt'), ...
    'Delimiter',' ','ReadVariableNames',false));
resp_funct_file  = table2array( readtable( ...
    strcat(model_dir,'raw2jpg_respFcns.txt'), ...
    'Delimiter',' ','ReadVariableNames',false));

% Color space transform
Ts             = transforms_file(1:3,:);

% White balance transform
Tw             = diag(transforms_file(8,:));

% Combined transforms
TsTw           = Ts*Tw;
TsTw_file      = transforms_file(5:7,:);

% Perform quick check to determine equivalence with provided model
% Round to nearest 4 decimal representation for check
TsTw_4dec      = round(TsTw*10000)/10000;
TsTw_file_4dec = round(TsTw_file*10000)/10000;
assert( isequal( TsTw_4dec, TsTw_file_4dec), ...
    'Transform multiplication not equal to result found in model file' ) 

% Gamut mapping: Control points
ctrl_points    = ctrl_points_file;

% Gamut mapping: Weights
weights        = coeficients_file(1:(size(coeficients_file,1)-4),:);

% Gamut mapping: c
c              = coeficients_file((size(coeficients_file,1)-3):end,:);

% Tone mapping
f              = resp_funct_file;

%==============================================================
% Import Backward Model Data
% BLANK FOR NOW

%==============================================================
% Import Raw Image Data

% NOTE: Can use RAW2TIFF.cpp to convert raw to tiff. This isn't
% automatically called by this script yet, but could be.

in_image = imread(strcat(image_dir,in_image_name));

%==============================================================
% Forward pipeline function

% Scale input image from 12 bit values to 16 bit values
% (multiply by 2^4)
in_scaled        = in_image * 16;

% Convert to uint16 representation
in_image_unit16  = im2uint16(in_scaled);

% Demosaic image
demosaiced       = demosaic(in_image_unit16,'gbrg');

% Cast to float for rest of processing
image_float      = im2double(demosaiced);

% Apply color space transform and white balance transform

% Better way to do this?
transformed      = image_float;
for y = 1:(size(image_float,1)/2)
    for x = 1:size(image_float,2)
        transformed(y,x,:) = transpose(squeeze(image_float(y,x,:))) * transpose(TsTw);
    end
end

out_image = transformed;

%==============================================================
% Export Image



imwrite(out_image,strcat(image_dir,in_image_name,'.output.tif'));
